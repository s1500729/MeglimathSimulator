﻿#pragma once
#include "Client.h"
#include "../MeglimathCore/Utility.h"
#include "../MeglimathCore/GameLogic/Think.h"
#include "GetEssentialStep.h"

class DoubleNextBestClient :
	public Client
{
public:
	DoubleNextBestClient();
	DoubleNextBestClient(TeamType type);
	
	int double_stop_cnt = 0;		// 自分のエージェント両方の行動が連続して何回失敗したかをカウントする
	const int DOUBLE_STOP_LIMIT = 2;		// 自分のエージェント両方の行動の失敗が連続して DOUBLE_STOP_LIMIT 回を超えた場合かつ劣勢時に妥協した手を打つ
	const int DOUBLE_STOP_LIMIT_FORCE = 5;		// 自分のエージェント両方の行動の失敗が連続して DOUBLE_STOP_LIMIT_FORCE 回を超えた場合ランダムな手を打つ
	static const int EXPLORE_DEPTH = 3;		// 探索の深さ = 何手先まで読むか

	_Point<> pos_history[2] = { _Point<>(), _Point<>() };		// 前のターン自分のエージェントがどこにいたか
	Array<Think> candidates[2];		// candidates[n] は (n+1)番目に優先される行動の候補リスト

	int eval_points[2] = { -100000, -100000 };		// eval_points[n] は (n+1)番目に優先される 1手後の行動 の評価値
	double eval_points_next[EXPLORE_DEPTH];		// eval_points_next[n] は (n+1)手後の行動 の評価値


	String Name() override
	{
		return U"DoubleNextBest";
	}

	Array<_Point<int>> GetNewPositionsFromSteps(Array<_Point<int>> positions, Array<Step> steps)
	{
		return { 
			positions[0] + DirectionToDeltaPos(steps[0].direction),
			positions[1] + DirectionToDeltaPos(steps[1].direction),
		};
	}

	/// <summary>
	/// depth手後に相手との得点差が最大になるように探索する再帰関数。
	/// </summary>
	/// <param name="info">現在のフィールドやエージェントの状態、チーム情報。再帰による値の変動なし</param>
	/// <param name="field">探索中のフィールドの状態。再帰による値の変動あり</param>
	/// <param name="depth">現在の状態から探索する残り手数。再帰による値の変動あり</param>
	/// <param name="positions">探索中の２人のエージェントの位置。再帰による値の変動あり</param>
	/// <param name="s1">探索１手目のエージェントの動き。再帰の最初の呼び出しでのみ値の変動あり</param>
	void Explore(const GameInfo& info, Field field, int depth, Array<_Point<int>> positions, Array<Step> s1)
	{
		auto agents = info.GetAgents(type);
		auto original = info.GetField();
		auto this_team = type;
		auto other_team = Transform::GetInverseTeam(this_team);

		auto all_step = Utility::AllStep();

		if (depth == 0)		// n手の探索の後処理
		{
			double eval_point_total = 0;		// eval_point_total は eval_points_next の総和, 次の一手の評価基準
			for (double e : eval_points_next)
				eval_point_total += e;

			if (!field.IsSameStateField(original))		// 自分のエージェント同士の衝突が検出されなければ最善手のリストを更新
			{
				for (int it = 0; it < 2; it++)
				{
					if (eval_points[it] <= eval_point_total)
					{
						if (eval_points[it] != eval_point_total)
							candidates[it].clear();

						eval_points[it] = eval_point_total;
						candidates[it].push_back({ s1[0],s1[1] });
						break;
					}
				}
			}
		}
		else
		{	
			// n手の探索
			for (auto step1 : GetEssentialStep(field, this_team, positions[0], EXPLORE_DEPTH))
			{
				auto next_field_a = field.MakeFieldFromStep(this_team, agents[0], step1);

				for (auto step2 : GetEssentialStep(field, this_team, positions[1], EXPLORE_DEPTH))
				{
					auto next_field_b = next_field_a.MakeFieldFromStep(this_team, agents[1], step2);

					auto points = next_field_b.GetTotalPoints();
					eval_points_next[depth - 1] = static_cast<double>(points[this_team] - points[other_team]);
					
					for (auto s : { step1,step2 })
					{
						if (s.action == Action::RemoveTile)
							eval_points_next[depth - 1] *= 1.2;
					}

					if (depth == EXPLORE_DEPTH)
						Explore(info, next_field_b, depth - 1, GetNewPositionsFromSteps(positions, { step1,step2 }), { step1,step2 });		// 最初のみ 仮引数 s1, s2 を更新
					else
						Explore(info, next_field_b, depth - 1, GetNewPositionsFromSteps(positions, { step1,step2 }), s1);
				}
			}
		}
	}

	void Update(const GameInfo& info) override
	{
		if (IsReady())
		{
			return;
		}

		auto agents = info.GetAgents(type);		
		auto field = info.GetField();
		auto this_team = type;
		auto other_team = Transform::GetInverseTeam(this_team);

		// 自分のチームのエージェント両方の行動が連続で失敗した数を数える
//要修正 条件 : [移動していない] -> [盤面に変更がない]
		if (!(pos_history[0] == agents[0].position)
			|| !(pos_history[1] == agents[1].position))
			double_stop_cnt = 0;
		else
			double_stop_cnt++;

		pos_history[0] = agents[0].position;
		pos_history[1] = agents[1].position;

		Think next_thinks[2] = {
			{
				Step{ Action(Random(0, 1)), Direction(Random(0, 7)) },
				Step{ Action(Random(0, 1)), Direction(Random(0, 7)) }
			},
			{
				Step{ Action(Random(0, 1)), Direction(Random(0, 7)) },
				Step{ Action(Random(0, 1)), Direction(Random(0, 7)) }
			}
		};		// next_thinks[n] は (n+1)番目に優先される行動

		for (int it = 0; it < 2; it++)
			eval_points[it] = -100000;

		for (auto c : candidates)
			c.clear();

		Explore(
			info,
			field,
			EXPLORE_DEPTH,
			{ agents[0].position,agents[1].position },
			{ {Action::Stop,Direction::Stop},{Action::Stop,Direction::Stop} }
		);		// 探索本体に現在地とダミーのStepを2つ渡す

		for (int it = 0; it < 2; it++)
			if (candidates[it].count() != (size_t)0)
				next_thinks[it] = Sample(candidates[it]);

		// 劣勢判断用変数 p_diff
		auto p_diff = field.GetTotalPoints()[this_team] - field.GetTotalPoints()[other_team];
		
		// 劣勢時に両方のエージェントが足止めされている場合、妥協して２番目に良い手を打つ
		if (double_stop_cnt > DOUBLE_STOP_LIMIT_FORCE && p_diff < 0)
		{
			auto es_0 = GetEssentialStep(field, this_team, agents[0].position, EXPLORE_DEPTH);
			auto es_1 = GetEssentialStep(field, this_team, agents[1].position, EXPLORE_DEPTH);

			_think =
			{
				es_0.size() == 0 ? Step{ Action(Random(0, 1)), Direction(Random(0, 7)) } : es_0[Random(es_0.size() - 1)],
				es_1.size() == 0 ? Step{ Action(Random(0, 1)), Direction(Random(0, 7)) } : es_1[Random(es_1.size() - 1)]
			};
		}
		else if (double_stop_cnt > DOUBLE_STOP_LIMIT && p_diff < 0)
			_think = next_thinks[1];
		else
			_think = next_thinks[0];

		_is_ready = true;
	}
};