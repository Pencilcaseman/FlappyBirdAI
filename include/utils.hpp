#pragma once

bool rectIntersection(const surge::Rectangle &a, const surge::Rectangle &b) {
	double x1 = a.pos().x();
	double y1 = a.pos().y();
	double w1 = a.size().x();
	double h1 = a.size().y();

	double x2 = b.pos().x();
	double y2 = b.pos().y();
	double w2 = b.size().x();
	double h2 = b.size().y();

	return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}
