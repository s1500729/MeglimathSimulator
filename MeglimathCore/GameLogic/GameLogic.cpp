﻿#include "GameLogic.h"
#include<random>
#include<set>
#include<map>
std::vector<Agent>  GameLogic::GetAgents() const
{
	std::vector<Agent>  ret{
		teams[0].agents[0],
		teams[0].agents[1],
		teams[1].agents[0],
		teams[1].agents[1]
	};
	return ret;
}

void GameLogic::initAgentsPos()
{
	_Size size = _field.cells.size();

	initAgentsPos({ (std::rand() >> 8) % static_cast<int>(size.x / 2) , (std::rand() >> 8) % static_cast<int>(size.y / 2) });
}

void GameLogic::initAgentsPos(_Point<> init_pos)
{
	_Size size = _field.cells.size() - _Size{ 1, 1 };

	// エージェントの初期位置のタイルを塗る
	initAgentPos({
		init_pos,
		_Point<int>(size.x - init_pos.x, init_pos.y),
		_Point<int>(init_pos.x, size.y - init_pos.y),
		size - init_pos
		});
}

void GameLogic::initAgentPos(std::array<_Point<>, 4> init_pos)
{
	_field.PaintCell(init_pos[0], TeamType::Blue);
	_field.PaintCell(init_pos[1], TeamType::Blue);

	_field.PaintCell(init_pos[2], TeamType::Red);
	_field.PaintCell(init_pos[3], TeamType::Red);

	teams[0].InitAgentsPos(init_pos[0], init_pos[1]);
	teams[1].InitAgentsPos(init_pos[2], init_pos[3]);
}

void GameLogic::InitalizeFromJson(const std::string json)
{
	rapidjson::Document document;
	document.Parse(json.data());

	_field = Field::makeFieldFromJson(json);

	// 二人分の初期位置を取得
	if (document.HasMember("InitPos")) {
		auto init_pos = document["InitPos"].GetArray();

		std::array<_Point<>, 4> pos_list;
		for (int i = 0; i < 4; i++)
		{
			pos_list[i] = _Point<int>{ init_pos[i].GetString() };
		}
		initAgentPos(pos_list);
	}
	else {
		initAgentsPos();
	}
	_turn = document["Turn"].GetInt();
}

void GameLogic::InitializeRandom(int turn, int height, int width)
{
	this->_turn = turn;
	this->_field = Field::makeFieldRandom( _Size{static_cast<size_t>(height),static_cast<size_t>(width)} );
	initAgentsPos();
}

void GameLogic::InitializeVariable(int turn, const Field & field, const std::array<TeamLogic, 2>& teams)
{
	this->_turn = turn;
	this->_field = field;
	this->teams = teams;
}


int GameLogic::GetTurn() const
{
	return _turn;
}

std::array<TeamLogic, 2> GameLogic::GetTeams() const
{
	return teams;
}

void GameLogic::NextTurn(const std::unordered_map<TeamType, Think> &_thinks)
{
	if (_turn <= 0)
	{
		return;
	}

	auto p_thinks = CollisionProccess(_thinks);

	struct Move {
		_Point<> target;
		_Point<> old_point;
		Step step;
		TeamType team;
		int id;
	};
	// シミュレーション
	std::vector<Move> point_map;
	for (auto team : { TeamType::Blue,TeamType::Red })
		for (int i : {0, 1}) {
			auto& step = p_thinks.at(team).steps[i];
			auto& agent = teams[team].agents[i];
			// 対象の座標
			_Point<int> old_pos = agent.position;
			_Point<int> new_pos = old_pos + Transform::DirToDelta(step.direction);
			point_map.push_back({ new_pos, old_pos, step, team, i });
		}

	bool ok = false;
	while (!ok) {
		ok = true;
		for (auto& p : point_map) {
			_Point <> pos = p.target;
			TileType our_tile = Transform::ToTile(p.team);
			TileType their_tile = Transform::GetInverseTile(our_tile);

			// 行動対象の重複しておらず、移動しないエージェントの現在位置とも重ならない
			if (std::count_if(point_map.cbegin(), point_map.cend(),
				[p](Move itr){
				return itr.target == p.target 
					|| itr.step.action!=Action::Move && itr.old_point==p.target; })
				!= 1)
			{
				auto duplicate = p.target;
				// 行動対象の重複した全エージェントを停留に
				for (auto& op : point_map) {
					if (op.target == duplicate) {
						op.step = { Action::Stop,Direction::Stop };
						op.target = op.old_point;
					}
				}

				// 初めからシミュレーションを再開
				ok = false; break;
			}

			// その座標がフィールド内であること
			// 移動なら、その座標に相手のタイルがないこと
			if (_field.IsInField(pos) == false
				|| p.step.action == Action::Move && _field.cells[pos].tile == their_tile)
			{
				// 停留に変更する
				p.step = { Action::Stop,Direction::Stop };
				p.target = p.old_point;

				// 初めからシミュレーションを再開
				ok = false; break;
			}
		}
	}
	//行動
	for (auto& p : point_map)
	{
		if (p.step.action == Action::Move) {
			// 進んだセルを塗る
			_field.PaintCell(p.target, p.team);
			// エージェントを動かす
			teams[p.team].MoveAgent(p.id, p.step.direction);
		}
		else if (p.step.action == Action::RemoveTile) {
			_field.RemoveTile(p.target);
		}
	}

	// ターンを進める
	_turn--;
}

bool GameLogic::IsThinkAble(TeamType team, Think think)const
{
	TileType our_tile = Transform::ToTile(team);
	TileType their_tile = Transform::GetInverseTile(our_tile);
	int i = 0;
	for (auto step : think.steps) {
		_Point pos = teams[team].agents[i].position + Transform::DirToDelta(step.direction);
		if (!_field.IsInField(pos))return false;
		if (step.action == Action::Move)
			if (_field.cells[pos].tile == their_tile)return false;
		else if (step.action == Action::RemoveTile)
			if (_field.cells[pos].tile == TileType::None)return false;
		i++;
	}
	return true;
}

bool GameLogic::GetGameEnd()
{
	return GetTurn() == 0;
}

int GameLogic::GetWinner()
{
	if (GetTurn() != 0)return -1;
	auto total_points = _field.GetTotalPoints();
	if (total_points[0] > total_points[1]) {
		return TeamType::Blue;
	}
	else if (total_points[0] < total_points[1]) {
		return TeamType::Red;
	}
	else {
		return -1;
	}
}

void GameLogic::SpinRight90()
{
	_field.SpinRight90();

	std::array<_Point<>, 4> inits;
	auto agents = GetAgents();
	for(int i = 0;i < 4;i++)
	{
		auto & p = agents[i].position;
		inits[i] = _Point<>(_field.cells.width() - 1 - p.y, p.x);
	}

	initAgentPos(inits);
}

void GameLogic::SpinLeft90()
{
	_field.SpinLeft90();

	std::array<_Point<>, 4> inits;
	auto agents = GetAgents();
	for (int i = 0; i < 4; i++)
	{
		auto & p = agents[i].position;
		inits[i] = _Point<>(p.y, p.x);
	}

	initAgentPos(inits);
}

std::unordered_map<TeamType, Think> GameLogic::CollisionProccess(const std::unordered_map<TeamType, Think>& _thinks)
{
	auto p_thinks = _thinks;

	for (auto team : { TeamType::Blue,TeamType::Red })
	{
		for (auto i : { 0,1 })
		{
			if (_thinks.at(team).steps[i].action == Action::Collision)
			{
				int idx = (int)_thinks.at(team).steps[i].direction;

				p_thinks[(TeamType)(1 - team)].steps[idx].action = Action::Stop;
				
				p_thinks[team].steps[i].action = Action::Stop;
			}
		}
	}

	return p_thinks;
}

const Field& GameLogic::GetField() const
{
	return _field;
}