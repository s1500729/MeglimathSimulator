#include "Team.h"


//int Team::GetTotalPoint() const
//{
//	return _total_point;
//}
//
//void Team::SetTotalPoint(int total_point)
//{
//	_total_point = total_point;
//}
//
//Array<Agent> Team::GetAgents() const
//{
//	return Array<Agent>({ _agents[0], _agents[1] });
//}
//
//void Team::MoveAgent(int idx, Direction dir)
//{
//	_agents[idx].Move(dir);
//}
//
//void Team::MoveAgent(Point pos, Direction dir)
//{
//	for (auto & agent : _agents)
//	{
//		if (agent.GetPosition() == pos)
//		{
//			agent.Move(dir);
//			return;
//		}
//	}
Team::Team(TeamLogic & team) :_team(team)
{
}


Team::~Team()
{
}