#include "Grenade.h"
#include <cmath>

const double PI = 3.14;
Grenade::Grenade(double xPos, double yPos, int team)
{
	int i;
	double angle, teta = (360 / NUM_BULLETS) * PI / 180;
	x = xPos;
	y = yPos;

	for (i = 0, angle = 0; i < NUM_BULLETS; i++, angle += teta)
	{
		bullets[i] = new Bullet(x, y, angle, team);
	}
	isExploded = false;
	this->team = team;
}

void Grenade::Explode(int map[MSZ][MSZ], NPC** team1, NPC** team2, double securityMap[MSZ][MSZ])
{
	const double MAX_RANGE = 20.0;
	bool anyBulletIsStillMoving = false; //explode is still ongoing

	for (int i = 0; i < NUM_BULLETS; i++)
	{
		if (bullets[i] && bullets[i]->getIsMoving())
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
				bullets[i]->Move(map, team1, team2, securityMap);
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
	int i;

	for (i = 0; i < NUM_BULLETS; i++)
	{
		bullets[i]->setIsMoving(value);
	}

}

void Grenade::Show()
{
	int i;

	for (i = 0; i < NUM_BULLETS; i++)
	{
		bullets[i]->Show();
	}

}