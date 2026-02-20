#include "NPC.h"
#include "AStar.h"
#include "Map.h"
#include "SecurityMap.h"
#include <cmath>


NPC::NPC(double positionX, double positionY, char character, int team , int type)
{
	x = positionX;
	y = positionY;
	symbol = character;
	this->team = team;
	hp = 7*MAX_HP/8;
	viewDistance = 30.0;
	this->npcType = type;
	directionX = 0.0;
	directionY = 0.0;
	isMoving = false;
	targetX = 0.0;
	targetY = 0.0;

	//init visibility map
	for (int i = 0; i < MSZ; i++) {
		for (int j = 0; j < MSZ; j++) {
			visibilityMap[i][j] = 0; 
		}
	}
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

	// label
	glColor3d(0, 0, 0);
	glRasterPos2d(x + 0.9, y + 0.5);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, symbol);
	// Show HP bar
	glLineWidth(2);

	// Normalize hp (assuming max HP is 1000 for example)
	double normalizedHP = hp / MAX_HP;

	// Interpolate between red (low HP) and green (full HP)
	double red = 1.0 - normalizedHP;  // more red when HP is low
	double green = normalizedHP;      // more green when HP is high
	double blue = 0.0;

	glColor3d(red, green, blue);

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
			map[new_si][new_sj] != STONE && map[new_si][new_sj] != TREE && map[new_si][new_sj] != WATER)
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

bool NPC::HasLineOfSight(double targetX, double targetY) const
{
	double startX = x + 1.5;
	double startY = y + 1.5;

	double currentX = startX;
	double currentY = startY;

	double dx = targetX - startX; 
	double dy = targetY - startY; 
	double distance = sqrt(dx * dx + dy * dy);

	if (distance < 1.0) {
		return true;
	}

	double stepX = (dx / distance)*1.2;
	double stepY = (dy / distance)*1.5;

	for (int i = 0; i < static_cast<int>(distance); ++i)
	{
		currentX += stepX;
		currentY += stepY;
		int mapX = static_cast<int>(currentX);
		int mapY = static_cast<int>(currentY);

		if (mapX < 0 || mapX >= MSZ || mapY < 0 || mapY >= MSZ) {
			return false;
		}

		if (map[mapX][mapY] == STONE || map[mapX][mapY] == TREE) {
			return false;
		}
	}
	return true;
}

void NPC::UpdateVisibility(NPC* myTeam[TEAM_SIZE], NPC* enemyTeam[TEAM_SIZE])
{
	for (int i = 0; i < MSZ; i++) {
		for (int j = 0; j < MSZ; j++) {
			visibilityMap[i][j] = 0;
		}
	}

	//scan enemy team (positive value)
	for (int i = 0; i < TEAM_SIZE; i++) {
		NPC* currentEnemy = enemyTeam[i];
		if (currentEnemy == nullptr) continue;

		double entityX, entityY;
		currentEnemy->getPosition(entityX, entityY);
		entityX += 1.5;
		entityY += 1.5;

		if (Distance(x, y, entityX, entityY) <= viewDistance && HasLineOfSight(entityX, entityY)) {
			int mapX = static_cast<int>(entityX);
			int mapY = static_cast<int>(entityY);
			if (mapX >= 0 && mapX < MSZ && mapY >= 0 && mapY < MSZ) {
				visibilityMap[mapX][mapY] = currentEnemy->getNpcType(); //positive value for enemy
			}
		}
	}

	//scan my team (negative value)
	for (int i = 0; i < TEAM_SIZE; i++) {
		NPC* currentTeammate = myTeam[i];
		if (currentTeammate == nullptr || currentTeammate == this) continue; //skip self

		double entityX, entityY;
		currentTeammate->getPosition(entityX, entityY);

		if (Distance(x, y, entityX, entityY) <= viewDistance && HasLineOfSight(entityX, entityY)) {
			int mapX = static_cast<int>(entityX);
			int mapY = static_cast<int>(entityY);
			if (mapX >= 0 && mapX < MSZ && mapY >= 0 && mapY < MSZ) {
				//negative value for teammate
				visibilityMap[mapX][mapY] = -currentTeammate->getNpcType();
			}
		}
	}
}


void NPC::DrawVisibilityMap() const
{
	int i, j;

	for (i = 0; i < MSZ; i++)
	{
		for (j = 0; j < MSZ; j++)
		{
			if (HasLineOfSight(i + 0.5, j + 0.5))
			{
				if (visibilityMap[i][j] != 0)
				{
					glColor3d(1.0, 1.0, 0.0);
					glBegin(GL_POLYGON);
					glVertex2d(i, j);
					glVertex2d(i, j + 1);
					glVertex2d(i + 1, j + 1);
					glVertex2d(i + 1, j);
					glEnd();
				}
				else
				{
					switch (map[i][j])
					{
					case 0:
						glColor3d(0.9, 0.9, 0.9);
						glBegin(GL_POLYGON);
						glVertex2d(i, j);
						glVertex2d(i, j + 1);
						glVertex2d(i + 1, j + 1);
						glVertex2d(i + 1, j);
						glEnd();
						break;

					case STONE:
						glColor3d(0.3, 0.3, 0.3);
						glBegin(GL_POLYGON);
						glVertex2d(i, j);
						glVertex2d(i, j + 1);
						glVertex2d(i + 1, j + 1);
						glVertex2d(i + 1, j);
						glEnd();
						break;

					case TREE:
						glColor3d(0.0, 0.6, 0.0);
						glBegin(GL_POLYGON);
						glVertex2d(i, j);
						glVertex2d(i, j + 1);
						glVertex2d(i + 1, j + 1);
						glVertex2d(i + 1, j);
						glEnd();
						break;

					case WATER:
						glColor3d(0.6, 0.8, 1.0);
						glBegin(GL_POLYGON);
						glVertex2d(i, j);
						glVertex2d(i, j + 1);
						glVertex2d(i + 1, j + 1);
						glVertex2d(i + 1, j);
						glEnd();
						break;
					case ARMORY_1:
					case ARMORY_2:
					case MEDICNE_1:
					case MEDICNE_2:
						glColor3d(1.0, 0.9, 0.2);
						glBegin(GL_POLYGON);
						glVertex2d(i, j);
						glVertex2d(i, j + 1);
						glVertex2d(i + 1, j + 1);
						glVertex2d(i + 1, j);
						glEnd();
						break;
					}
				}
			}
			else
			{
				glColor3d(0.1, 0.1, 0.1);
				glBegin(GL_POLYGON);
				glVertex2d(i, j);
				glVertex2d(i, j + 1);
				glVertex2d(i + 1, j + 1);
				glVertex2d(i + 1, j);
				glEnd();
			}
		}
	}
}

void NPC::setPath(const std::vector<std::pair<int, int>>& newPath) {
	path = newPath;
	// Start at index 1 since index 0 is the current location
	pathIndex = newPath.size() > 1 ? 1 : -1;
}

bool NPC::getIsMoving() const {
	return isMoving;
}

int NPC::getPathIndex() const {
	return pathIndex;
}