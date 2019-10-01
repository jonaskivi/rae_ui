#include "rae/core/version.hpp"

#include "rae/core/ScreenInfo.hpp"

#include "rae/core/Math.hpp"

#ifdef version_catch
#include "rae/core/catch.hpp"

SCENARIO("ScreenInfo unittest", "[rae][ScreenInfo]")
{
	//2560 x 1600

	GIVEN( "A ScreenInfo of size 2560 x 1600" )
	{

		rae::ScreenInfo screenInfo(0, 2560, 1600, 2560, 1600);

		REQUIRE(screenInfo.screenNumber() == 0);
		REQUIRE(screenInfo.screenWidthP() == 2560);
		REQUIRE(screenInfo.screenHeightP() == 1600);

		//2560 / 1600 = 1.6
		REQUIRE(screenInfo.screenAspect() == 1.6f);
		REQUIRE(screenInfo.screenHeight() == 1.0f);
		REQUIRE(screenInfo.screenWidth() == 1.6f);

		//REQUIRE(screenInfo.heightToPixels(1.0f) == 1600.0f);
		//REQUIRE(screenInfo.heightToPixels(0.5f) == 800.0f);
		//REQUIRE(screenInfo.heightToPixels(0.0f) == 0.0f);

		REQUIRE( rae::Math::isEqual( screenInfo.heightToPixels(1.0f), 1600.0f ) );
		REQUIRE( rae::Math::isEqual( screenInfo.heightToPixels(0.5f), 800.0f ) );
		REQUIRE( rae::Math::isEqual( screenInfo.heightToPixels(0.0f), 0.0f ) );

		REQUIRE( rae::Math::isEqual( screenInfo.pixelsToHeight(2560.0f), 1.6f ) );
		REQUIRE( rae::Math::isEqual( screenInfo.pixelsToHeight(1600.0f), 1.0f ) );
		REQUIRE( rae::Math::isEqual( screenInfo.pixelsToHeight(800.0f), 0.5f ) );
		REQUIRE( rae::Math::isEqual( screenInfo.pixelsToHeight(0.0f), 0.0f ) );

		REQUIRE( rae::Math::isEqual( screenInfo.percentToWidth(1.0f), 1.6f ) );
		REQUIRE( rae::Math::isEqual( screenInfo.percentToWidth(0.5f), 0.8f ) );
		REQUIRE( rae::Math::isEqual( screenInfo.percentToWidth(0.0f), 0.0f ) );

		WHEN( "the size is changed to 1280 x 800" )
		{
			screenInfo.setScreenSizeP(1280, 800);

			THEN( "all is still ok." )
			{
				//1280 / 800 = 1.6
				REQUIRE(screenInfo.screenAspect() == 1.6f);
				REQUIRE(screenInfo.screenHeight() == 1.0f);
				REQUIRE(screenInfo.screenWidth() == 1.6f);

				REQUIRE( rae::Math::isEqual( screenInfo.heightToPixels(1.0f), 800.0f ) );
				REQUIRE( rae::Math::isEqual( screenInfo.heightToPixels(0.5f), 400.0f ) );
				REQUIRE( rae::Math::isEqual( screenInfo.heightToPixels(0.0f), 0.0f ) );

				REQUIRE( rae::Math::isEqual( screenInfo.pixelsToHeight(1280.0f), 1.6f ) );
				REQUIRE( rae::Math::isEqual( screenInfo.pixelsToHeight(800.0f), 1.0f ) );
				REQUIRE( rae::Math::isEqual( screenInfo.pixelsToHeight(400.0f), 0.5f ) );
				REQUIRE( rae::Math::isEqual( screenInfo.pixelsToHeight(0.0f), 0.0f ) );

				REQUIRE( rae::Math::isEqual( screenInfo.percentToWidth(1.0f), 1.6f ) );
				REQUIRE( rae::Math::isEqual( screenInfo.percentToWidth(0.5f), 0.8f ) );
				REQUIRE( rae::Math::isEqual( screenInfo.percentToWidth(0.0f), 0.0f ) );
			}
		}

		WHEN( "the size is changed to 4000 x 4000" )
		{
			screenInfo.setScreenSizeP(4000, 4000);

			THEN( "all is still ok." )
			{
				//4000 / 4000 = 1.0
				REQUIRE(screenInfo.screenAspect() == 1.0f);
				REQUIRE(screenInfo.screenHeight() == 1.0f);
				REQUIRE(screenInfo.screenWidth() == 1.0f);

				REQUIRE( rae::Math::isEqual( screenInfo.heightToPixels(1.0f), 4000.0f ) );
				REQUIRE( rae::Math::isEqual( screenInfo.heightToPixels(0.5f), 2000.0f ) );
				REQUIRE( rae::Math::isEqual( screenInfo.heightToPixels(0.0f), 0.0f ) );

				REQUIRE( rae::Math::isEqual( screenInfo.pixelsToHeight(4000.0f), 1.0f ) );
				REQUIRE( rae::Math::isEqual( screenInfo.pixelsToHeight(8000.0f), 2.0f ) );
				REQUIRE( rae::Math::isEqual( screenInfo.pixelsToHeight(2000.0f), 0.5f ) );
				REQUIRE( rae::Math::isEqual( screenInfo.pixelsToHeight(0.0f), 0.0f ) );

				REQUIRE( rae::Math::isEqual( screenInfo.percentToWidth(1.0f), 1.0f ) );
				REQUIRE( rae::Math::isEqual( screenInfo.percentToWidth(0.5f), 0.5f ) );
				REQUIRE( rae::Math::isEqual( screenInfo.percentToWidth(0.0f), 0.0f ) );
			}
		}
	}
}
#endif

using namespace rae;

ScreenInfo::ScreenInfo()
{
	calculateScreenAspect();
	calculateHalfScreens();

	setDpi(96.0f);

	printInfo();
}

ScreenInfo::ScreenInfo(
	int screenNumber,
	int screenWidthP,
	int screenHeightP,
	int visibleAreaWidthP,
	int visibleAreaHeightP) :
		m_screenNumber(screenNumber)
{
	//screenHeightP(screenHeightP);
	//screenWidthP(screenWidthP);
	m_screenWidthP = screenWidthP;
	
	// updateScreenHeightStuff:
	m_pixelsToHeight = 1.0f / (float)screenHeightP;
	m_heightToPixels = (float)screenHeightP;
	m_screenHeightP = screenHeightP;

	m_visibleAreaWidthP = visibleAreaWidthP;
	m_visibleAreaHeightP = visibleAreaHeightP;

	calculateScreenAspect();
	calculateHalfScreens();

	setDpi(96.0);

	printInfo();
}

void ScreenInfo::setScreenSizeP(int set_widthP, int set_heightP)
{
	m_screenWidthP = set_widthP;

	// updateScreenHeightStuff:
	m_pixelsToHeight = 1.0f / (float)set_heightP;
	m_heightToPixels = (float)set_heightP;
	m_screenHeightP = set_heightP;

	calculateScreenAspect();
	calculateHalfScreens();
}

void ScreenInfo::screenWidthP(int set)
{
	m_screenWidthP = set;
	calculateScreenAspect();
	calculateHalfScreens();
}

void ScreenInfo::screenHeightP(int set)
{
	#ifdef DebugRae
		RaeLog << "ScreenInfo::screenHeightP() set to:" << set << "\n";
	#endif
	
	// updateScreenHeightStuff:
	m_pixelsToHeight = 1.0f / (float)set;
	m_heightToPixels = (float)set;
	m_screenHeightP = set;

	calculateScreenAspect();
	calculateHalfScreens();
}

void ScreenInfo::setDpi(float set)
{
	// The range for the Dpi is currently 30-300.
	// You'll get distortions in both ends. So, the
	// real usable range is something about 60-180.
	if( set < 30.0f ) set = 30.0f;
	else if( set > 300.0f ) set = 300.0f;

	m_dpiMul = set / 96.0f;

	// Adjust curveSideSize to fall on the desired range.
	// Dpi over about 190.0 will propably not look so good anymore.
	// even with these adjustments. Would have to change the texture
	// size to 128 in that case. TODO.
	m_curveSideSize = m_dpiMul * 0.3f;
	if( m_curveSideSize < 0.25f ) m_curveSideSize = 0.25f;
	else if( m_curveSideSize > 0.48f ) m_curveSideSize = 0.48f;

	m_dpi = set;
	//m_dpiForSaving = set; // So that it will be saved too.
}

void ScreenInfo::calculatePixelsPerMM()
{
	if (m_widthMM > 0.0f)
	{
		m_pixelsPerMM = float(m_screenWidthP) / m_widthMM;

		setDpi(float(m_screenWidthP) / (m_widthMM / 25.4f));
	}
}
