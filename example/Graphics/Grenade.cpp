#include "Grenade.h"
#include "glut.h"
#include <cmath>

const double PI = 3.14159265358979;

Grenade::Grenade(double startX_, double startY_, double targetX_, double targetY_, int team_)
{
	startX = startX_;
	startY = startY_;
	targetX = targetX_;
	targetY = targetY_;
	x = startX;
	y = startY;
	team = team_;
	isExploded = false;
	spawnTimeMs = glutGet(GLUT_ELAPSED_TIME);
	for (int i = 0; i < NUM_BULLETS; i++)
		bullets[i] = nullptr;
}

Grenade::~Grenade()
{
	for (int i = 0; i < NUM_BULLETS; i++) {
		if (bullets[i] != nullptr) {
			delete bullets[i];
			bullets[i] = nullptr;
		}
	}
}

void Grenade::createFragments()
{
	double teta = (2.0 * PI) / NUM_BULLETS;
	for (int i = 0; i < NUM_BULLETS; i++) {
		double angle = i * teta;
		bullets[i] = new Bullet(x, y, angle, team, 0.5 * MAX_HP);
	}
}

void Grenade::Update()
{
	if (isExploded) return;
	int elapsed = glutGet(GLUT_ELAPSED_TIME) - spawnTimeMs;
	if (elapsed >= GRENADE_FUSE_MS) {
		createFragments();
		setIsExploded(true);
		return;
	}
	double t = (double)elapsed / (double)GRENADE_FUSE_MS;
	if (t > 1.0) t = 1.0;
	x = startX + (targetX - startX) * t;
	y = startY + (targetY - startY) * t + 8.0 * 4.0 * t * (1.0 - t);
}

void Grenade::Explode(int map[MSZ][MSZ], NPC** team1, NPC** team2, double securityMap[MSZ][MSZ])
{
	const double MAX_RANGE = 20.0;
	bool anyBulletIsStillMoving = false; //explode is still ongoing

	bool hitTeam1[TEAM_SIZE] = { false };
	bool hitTeam2[TEAM_SIZE] = { false };

	for (int i = 0; i < NUM_BULLETS; i++)
	{
		if (bullets[i] == nullptr) continue;
		if (bullets[i]->getIsMoving())
		{
			double bulletX = bullets[i]->getX();
			double bulletY = bullets[i]->getY();

			//distance from grenade center to bullet
			double dx = bulletX - this->x;
			double dy = bulletY - this->y;
			double distance = std::sqrt(dx * dx + dy * dy);

			if (distance > MAX_RANGE)
			{
				bullets[i]->setIsMoving(false);
			}
			else
			{
				bullets[i]->Move(map, team1, team2, securityMap, hitTeam1, hitTeam2);
				if (bullets[i]->getIsMoving())
				{
					anyBulletIsStillMoving = true;
				}
			}
		}
	}
	if (!anyBulletIsStillMoving)
	{
		this->isExploded = false;
	}
}

bool Grenade::getIsExploded() const {
	return isExploded;
}

void Grenade::setIsExploded(bool value)
{
	isExploded = value;
	for (int i = 0; i < NUM_BULLETS; i++) {
		if (bullets[i] != nullptr)
			bullets[i]->setIsMoving(value);
	}
}

void Grenade::Show()
{
	if (!isExploded) {
		glColor3d(0.2, 0.5, 0.2);
		glBegin(GL_QUADS);
		glVertex2d(x - 0.4, y - 0.4);
		glVertex2d(x + 0.4, y - 0.4);
		glVertex2d(x + 0.4, y + 0.4);
		glVertex2d(x - 0.4, y + 0.4);
		glEnd();
		return;
	}
	for (int i = 0; i < NUM_BULLETS; i++) {
		if (bullets[i] != nullptr)
			bullets[i]->Show();
	}
}