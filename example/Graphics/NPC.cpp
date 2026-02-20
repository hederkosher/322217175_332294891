#include "NPC.h"
#include "AStar.h"
#include "Map.h"
#include "SecurityMap.h"
#include <cmath>
#include <iostream>


NPC::NPC(double positionX, double positionY, char character, int team, int type)
{
	x = positionX;
	y = positionY;
	symbol = character;
	this->team = team;
	hp = 7 * MAX_HP / 8;
	viewDistance = 30.0;
	this->npcType = type;
	directionX = 0.0;
	directionY = 0.0;
	isMoving = false;
	targetX = 0.0;
	targetY = 0.0;

	// Generate random personality
	aggressiveness = 0.3 + (rand() % 7) * 0.1; // 0.3 to 0.9
	cautiousness = 0.3 + (rand() % 7) * 0.1;   // 0.3 to 0.9

	std::string color = (team == 1 ? TEAM1 : TEAM2);
	std::cout << color << "NPC [" << character << "] Team " << team
		<< " personality: aggression=" << aggressiveness
		<< " caution=" << cautiousness << RESET << std::endl;

	for (int i = 0; i < MSZ; i++)
		for (int j = 0; j < MSZ; j++)
			visibilityMap[i][j] = 0;
}

bool NPC::isInRisk() const
{
	return hp < MAX_HP / 2.0;
}


double NPC::Distance(double x1, double y1, double x2, double y2)
{
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}


void NPC::setMovingDirection()
{
	double length;
	directionX = targetX - x;
	directionY = targetY - y;
	length = sqrt(directionX * directionX + directionY * directionY);
	if (length < 0.001) return;
	directionX /= length;
	directionY /= length;
}

void NPC::show()
{
	double size = 3.0;

	if (team == 1) glColor3d(1.0, 0.5, 0.0);
	else           glColor3d(0.0, 0.6, 1.0);

	glBegin(GL_QUADS);
	glVertex2d(x, y);
	glVertex2d(x + size, y);
	glVertex2d(x + size, y + size);
	glVertex2d(x, y + size);
	glEnd();

	glColor3d(0, 0, 0);
	glRasterPos2d(x + 0.9, y + 0.5);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, symbol);

	// HP bar
	glLineWidth(2);
	double normalizedHP = hp / MAX_HP;
	double red = 1.0 - normalizedHP;
	double green = normalizedHP;
	glColor3d(red, green, 0.0);

	glBegin(GL_LINES);
	glVertex2d(x, y + 3.5);
	glVertex2d(x + hp * 0.003, y + 3.5);
	glEnd();

	glLineWidth(1);
}

void NPC::setIsMoving(bool value) { isMoving = value; }
void NPC::setTarget(double tx, double ty) { targetX = tx; targetY = ty; }

int NPC::getTeam() { return team; }
void NPC::setCurrentState(State* s) { pCurrentState = s; }
State* NPC::getCurrentState() { return pCurrentState; }
void NPC::getPosition(double& px, double& py) { px = x; py = y; }
double NPC::getHp() { return hp; }
void NPC::setHp(double value) { hp = value; }

int NPC::getNpcType() { return npcType; }

bool NPC::getIsGettingHp() { return isGettingHp; }
void NPC::setIsGettingHp(bool value) { isGettingHp = value; }

int (*NPC::getVisibilityMap())[MSZ] {
	return visibilityMap;
}

int NPC::getCurrentRoom() const {
	return GetRoomAt(x + 1.5, y + 1.5);
}

bool NPC::PlanPathTo() {
	int ti = int(targetX);
	int tj = int(targetY);
	int si = int(x);
	int sj = int(y);
	std::vector<std::pair<int, int>> p;

	if (FindPath(si, sj, ti, tj, p)) {
		path.swap(p);
		pathIndex = (path.size() > 1) ? 1 : -1;
		return true;
	}

	int escape_moves[4][2] = { {-3, 0}, {0, 3}, {0, -3}, {3, 0} };
	for (auto& move : escape_moves) {
		int new_si = si + move[0];
		int new_sj = sj + move[1];
		if (new_si >= 0 && new_si < MSZ && new_sj >= 0 && new_sj < MSZ &&
			map[new_si][new_sj] != WALL && map[new_si][new_sj] != STONE)
		{
			path.clear();
			path.push_back({ si, sj });
			path.push_back({ new_si, new_sj });
			pathIndex = 1;
			return true;
		}
	}

	path.clear();
	pathIndex = -1;
	return false;
}

bool NPC::FollowPlannedPath(double minDist) {
	if (pathIndex < 0 || pathIndex >= (int)path.size()) return false;

	int gi = path[pathIndex].first;
	int gj = path[pathIndex].second;

	targetX = gi + 0.5;
	targetY = gj + 0.5;

	setMovingDirection();

	x += directionX * SPEED;
	y += directionY * SPEED;

	double dx = targetX - x, dy = targetY - y;
	double dist = std::sqrt(dx * dx + dy * dy);
	if (dist < minDist) {
		pathIndex++;
		if (pathIndex >= (int)path.size()) {
			pathIndex = -1;
			return true;
		}
	}
	return false;
}

void NPC::DrawPath() const {
	if (path.empty()) return;

	glLineWidth(2.0f);
	glColor3d(1.0, 1.0, 0.0);
	glBegin(GL_LINE_STRIP);
	for (const auto& cell : path) {
		double vx = cell.first + 0.5;
		double vy = cell.second + 0.5;
		glVertex2d(vx, vy);
	}
	glEnd();

	glPointSize(5.0f);
	glBegin(GL_POINTS);
	for (const auto& cell : path) {
		double vx = cell.first + 0.5;
		double vy = cell.second + 0.5;
		glVertex2d(vx, vy);
	}
	glEnd();
}

bool NPC::HasLineOfSight(double tX, double tY) const
{
	double startX = x + 1.5;
	double startY = y + 1.5;
	double currentX = startX;
	double currentY = startY;

	double dx = tX - startX;
	double dy = tY - startY;
	double distance = sqrt(dx * dx + dy * dy);

	if (distance < 1.0) return true;

	double stepX = (dx / distance) * 1.2;
	double stepY = (dy / distance) * 1.5;

	for (int i = 0; i < static_cast<int>(distance); ++i)
	{
		currentX += stepX;
		currentY += stepY;
		int mapX = static_cast<int>(currentX);
		int mapY = static_cast<int>(currentY);

		if (mapX < 0 || mapX >= MSZ || mapY < 0 || mapY >= MSZ)
			return false;

		if (map[mapX][mapY] == WALL || map[mapX][mapY] == STONE)
			return false;
	}
	return true;
}

void NPC::UpdateVisibility(NPC* myTeamArr[TEAM_SIZE], NPC* enemyTeamArr[TEAM_SIZE])
{
	for (int i = 0; i < MSZ; i++)
		for (int j = 0; j < MSZ; j++)
			visibilityMap[i][j] = 0;

	for (int i = 0; i < TEAM_SIZE; i++) {
		NPC* currentEnemy = enemyTeamArr[i];
		if (currentEnemy == nullptr) continue;

		double entityX, entityY;
		currentEnemy->getPosition(entityX, entityY);
		entityX += 1.5;
		entityY += 1.5;

		if (Distance(x, y, entityX, entityY) <= viewDistance && HasLineOfSight(entityX, entityY)) {
			int mapX = static_cast<int>(entityX);
			int mapY = static_cast<int>(entityY);
			if (mapX >= 0 && mapX < MSZ && mapY >= 0 && mapY < MSZ)
				visibilityMap[mapX][mapY] = currentEnemy->getNpcType();
		}
	}

	for (int i = 0; i < TEAM_SIZE; i++) {
		NPC* currentTeammate = myTeamArr[i];
		if (currentTeammate == nullptr || currentTeammate == this) continue;

		double entityX, entityY;
		currentTeammate->getPosition(entityX, entityY);

		if (Distance(x, y, entityX, entityY) <= viewDistance && HasLineOfSight(entityX, entityY)) {
			int mapX = static_cast<int>(entityX);
			int mapY = static_cast<int>(entityY);
			if (mapX >= 0 && mapX < MSZ && mapY >= 0 && mapY < MSZ)
				visibilityMap[mapX][mapY] = -currentTeammate->getNpcType();
		}
	}
}


void NPC::DrawVisibilityMap() const
{
	for (int i = 0; i < MSZ; i++)
	{
		for (int j = 0; j < MSZ; j++)
		{
			if (HasLineOfSight(i + 0.5, j + 0.5))
			{
				if (visibilityMap[i][j] != 0)
				{
					glColor3d(1.0, 1.0, 0.0);
				}
				else
				{
					switch (map[i][j])
					{
					case FLOOR:
						glColor3d(0.9, 0.9, 0.9);
						break;
					case WALL:
						glColor3d(0.2, 0.2, 0.25);
						break;
					case STONE:
						glColor3d(0.45, 0.42, 0.38);
						break;
					case ARMORY:
						glColor3d(0.9, 0.75, 0.1);
						break;
					case MEDICINE:
						glColor3d(0.9, 0.2, 0.3);
						break;
					default:
						glColor3d(0.9, 0.9, 0.9);
						break;
					}
				}
			}
			else
			{
				glColor3d(0.1, 0.1, 0.1);
			}

			glBegin(GL_POLYGON);
			glVertex2d(i, j);
			glVertex2d(i, j + 1);
			glVertex2d(i + 1, j + 1);
			glVertex2d(i + 1, j);
			glEnd();
		}
	}
}

void NPC::setPath(const std::vector<std::pair<int, int>>& newPath) {
	path = newPath;
	pathIndex = newPath.size() > 1 ? 1 : -1;
}

bool NPC::getIsMoving() const {
	return isMoving;
}

int NPC::getPathIndex() const {
	return pathIndex;
}
