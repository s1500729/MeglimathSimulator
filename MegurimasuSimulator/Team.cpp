#include "Team.h"

Array<Agent> Team::GetAgents() const
{
	return Array<Agent>({ _agents[0], _agents[1] });
}

void Team::MoveAgent(int idx, Direction dir)
{
	_agents[idx].Move(dir);
}

void Team::MoveAgent(Point pos, Direction dir)
{
	for (auto & agent : _agents)
	{
		if (agent.GetPosition() == pos)
		{
			agent.Move(dir);
			return;
		}
	}
}

Team::Team()
	:Team(TeamType::A, Agent(), Agent())
{}

Team::Team(TeamType type, Agent agent1, Agent agent2)
{
	_agents[0] = agent1;
	_agents[1] = agent2;

	_type = type;
	_total_point = 0;
}

Team::~Team()
{
}