#pragma once
#include "Field.h"
#include <Siv3D.hpp>

class FieldDrawer
{
private:
	Point _origin;

	/// <summary>
	/// セル一つ当たりの大きさ
	/// </summary>
	const Size _c_size = Size(32, 32);

public:
	void SetOrigin(Point origin);
	Point GetOrigin() const;

	void Draw(const Field & field) const;

public:
	FieldDrawer();
	~FieldDrawer();

	void operator=(const FieldDrawer& other);
};

