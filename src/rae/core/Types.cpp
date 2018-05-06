#include "rae/core/Types.hpp"

using namespace rae;

Color Colors::red		= Color(1,0,0,1);
Color Colors::green		= Color(0,1,0,1);
Color Colors::blue		= Color(0,0,1,1);
Color Colors::yellow	= Color(1,1,0,1);
Color Colors::magenta	= Color(1,0,1,1);
Color Colors::orange	= Color(1.0f, 0.647f, 0.0f, 1.0f);
Color Colors::cyan		= Color(0,1,1,1);
Color Colors::black		= Color(0,0,0,1);
Color Colors::white		= Color(1,1,1,1);
Color Colors::lightGray	= Color(0.75f, 0.75f, 0.75f, 1.0f);
Color Colors::gray		= Color(0.5f, 0.5f, 0.5f, 1.0f);
Color Colors::darkGray	= Color(0.25f, 0.25f, 0.25f, 1.0f);

Pivot Pivots::Center	= Pivot( 0,  0,  0);
Pivot Pivots::TopLeft2D	= Pivot(-1, -1,  0);
