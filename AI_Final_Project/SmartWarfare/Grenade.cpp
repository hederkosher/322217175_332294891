#include "Grenade.h"
#include "Definitions.h"
#include <cmath>
#include "glut.h"

const double PI = 3.14159265;

void Grenade::CalcArcPos(double t, double &outX, double &outY) const
{
	double lx = originX + (x - originX) * t;
	double ly = originY + (y - originY) * t;
	double offset = 4.0 * arcHeight * t * (1.0 - t);
	outX = lx + perpX * offset;
	outY = ly + perpY * offset;
}

Grenade::Grenade(double srcX, double srcY, double tgtX, double tgtY, int team)
{
	originX = srcX;
	originY = srcY;
	x = tgtX;
	y = tgtY;
	this->team = team;

	double dx = x - originX;
	double dy = y - originY;
	double dist = sqrt(dx * dx + dy * dy);

	perpX = (dist > 0.01) ? -dy / dist : 0.0;
	perpY = (dist > 0.01) ?  dx / dist : 0.0;

	arcHeight = (dist < 1.0) ? 0.0 : fmin(dist * 0.35, 10.0);

	flightDuration = (int)fmax(30.0, fmin(dist * 1.5, 60.0));
	flightProgress = 0.0;
	isFlying = true;
	fuseTimer = 0;

	currentX = srcX;
	currentY = srcY;

	arcCount = 0;

	int i;
	double angle, teta = (360.0 / NUM_BULLETS) * PI / 180.0;
	for (i = 0, angle = 0; i < NUM_BULLETS; i++, angle += teta)
	{
		bullets[i] = new Bullet(x, y, angle, team, GRENADE_BULLET_DAMAGE, 1.0, true);  // pierce through enemies
	}
	isExploded = false;
}

void Grenade::Update()
{
	if (isFlying)
	{
		flightProgress += 1.0 / flightDuration;
		if (flightProgress >= 1.0)
		{
			flightProgress = 1.0;
			isFlying = false;
			fuseTimer = 240;
			currentX = x;
			currentY = y;
		}
		else
		{
			CalcArcPos(flightProgress, currentX, currentY);
		}

		if (arcCount < ARC_POINTS)
		{
			arcX[arcCount] = currentX;
			arcY[arcCount] = currentY;
			arcCount++;
		}
	}
	else if (fuseTimer > 0)
	{
		fuseTimer--;
		if (fuseTimer <= 0)
		{
			setIsExploded(true);
		}
	}
}

void Grenade::Explode(int map[MSZ][MSZ], NPC** team1, NPC** team2, double securityMap[MSZ][MSZ])
{
	const double MAX_RANGE = 20.0;
	bool anyBulletIsStillMoving = false;

	for (int i = 0; i < NUM_BULLETS; i++)
	{
		if (bullets[i] && bullets[i]->getIsMoving())
		{
			double bulletX = bullets[i]->getX();
			double bulletY = bullets[i]->getY();

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
	for (int i = 0; i < NUM_BULLETS; i++)
	{
		bullets[i]->setIsMoving(value);
	}
}

void Grenade::Show()
{
	if (isFlying || fuseTimer > 0)
	{
		double r, g, b;
		if (team == 1) { r = 1.0; g = 0.5; b = 0.0; }
		else           { r = 0.0; g = 0.6; b = 1.0; }

		if (arcCount > 1)
		{
			glLineWidth(1.5f);
			glBegin(GL_LINE_STRIP);
			for (int i = 0; i < arcCount; i++)
			{
				double alpha = (double)(i + 1) / arcCount;
				glColor3d(r * alpha * 0.6, g * alpha * 0.6, b * alpha * 0.6);
				glVertex2d(arcX[i], arcY[i]);
			}
			glEnd();
			glLineWidth(1.0f);
		}

		if (fuseTimer > 0)
		{
			glPointSize(6.0f);
			double drawX = x, drawY = y;

			if (arcCount >= 3)
			{
				glColor3d(r * 0.3, g * 0.3, b * 0.3);
				glBegin(GL_LINE_STRIP);
				for (int i = 0; i < arcCount; i++)
				{
					double a = (double)(i + 1) / arcCount;
					glColor3d(r * a * 0.5, g * a * 0.5, b * a * 0.5);
					glVertex2d(arcX[i], arcY[i]);
				}
				glEnd();
			}

			bool blink = (fuseTimer % 8) < 4;
			if (blink)
				glColor3d(1.0, 0.2, 0.0);
			else
				glColor3d(1.0, 1.0, 0.0);

			glBegin(GL_POINTS);
			glVertex2d(drawX, drawY);
			glEnd();

			glPointSize(3.0f);
			glColor3d(1.0, 1.0, 1.0);
			glBegin(GL_POINTS);
			glVertex2d(drawX, drawY);
			glEnd();

			glPointSize(1.0f);
		}
		else
		{
			double scale = 1.0 + 2.0 * sin(flightProgress * PI);
			double sz = 3.0 + scale;
			glPointSize((float)sz);
			glColor3d(r, g, b);
			glBegin(GL_POINTS);
			glVertex2d(currentX, currentY);
			glEnd();

			glPointSize((float)(sz * 0.5));
			glColor3d(1.0, 1.0, 0.8);
			glBegin(GL_POINTS);
			glVertex2d(currentX, currentY);
			glEnd();

			glPointSize(1.0f);
		}
	}
	else if (isExploded)
	{
		for (int i = 0; i < NUM_BULLETS; i++)
		{
			bullets[i]->Show();
		}
	}
}
