#include "Game.h"

// TODO: �G�[�W�F���g���m���d�Ȃ�Ȃ��悤�ɂ���

GameInfo Game::getGameInfo() const
{
	return GameInfo(_field, getAgentMap());
}

std::map<TeamType, Array<Agent>> Game::getAgentMap() const
{
	std::map<TeamType, Array<Agent>> agents;
	agents[TeamType::A] = _teams[0]->GetAgents();
	agents[TeamType::B] = _teams[1]->GetAgents();

	return agents;
}

void Game::Update()
{
	std::map<TeamType, Think> thinks;
	
	GameInfo info = getGameInfo();
	auto agents = getAgentMap();

	thinks[TeamType::A] = _teams[0]->NextThink(info);
	thinks[TeamType::B] = _teams[1]->NextThink(info);

	// �V�~�����[�V����
	Array<std::pair<Point, std::pair<Direction, TeamType>>> move_point_arr;
	Array<Point> remove_points;
	for (TeamType team : {TeamType::A, TeamType::B})
	{
		for (int i = 0; i < 2; i++)
		{
			Direction dir = thinks[team].agents[i].direction;
			// �G�[�W�F���g�𓮂������������ɓ��������ꍇ�̍��W
			Point pos = agents[team][i].GetPosition().movedBy(Transform::DirToDelta(dir));

			// �G�[�W�F���g�����삷����W��ǉ�
			switch (thinks[team].agents[i].action)
			{
			case Action::Move:
				move_point_arr.push_back(std::make_pair(pos, std::make_pair(dir, team)));
				break;
			case Action::RemoveTile:
				remove_points.push_back(pos);
				break;
			}
		}
	}

	// �Փ˂��Ă��Ȃ��G�[�W�F���g�̍s���̂ݎ��s����
	for (auto & pos_map : move_point_arr)
	{
		auto pos = pos_map.first;

		if (move_point_arr.count_if([pos](std::pair<Point, std::pair<Direction, TeamType>> itr) {return itr.first == pos; }) == 1
			&& _field.IsInField(pos))
		{
			auto dir = pos_map.second.first;
			auto team = pos_map.second.second;

			// ���̍��W�ɖ߂�
			pos -= Transform::DirToDelta(dir);

			_teams[static_cast<int>(team)]->MoveAgent(pos, dir);
			_field.PaintCell(pos, team);
		}
	}

	for (auto & remove_point : remove_points)
	{
		if (remove_points.count_if(Equal(remove_point)) == 1 && _field.IsInField(remove_point))
		{
			_field.RemoveTile(remove_point);
		}
	}
	
}

void Game::Draw() const
{
	_field_drawer.Draw(getGameInfo());
}

Game::Game(const Field &field, std::shared_ptr<Team> team_a, std::shared_ptr<Team> team_b)
{
	_field = field;
	_teams.append({ team_a, team_b });
}

Game::~Game()
{
}