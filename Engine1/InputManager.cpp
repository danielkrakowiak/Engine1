#include "InputManager.h"

#include <windows.h>

using namespace Engine1;

InputManager::InputManager() {
	for ( int i = 0; i < keyboardKeyCount; ++i ) {
		keyboardButtonState.at( i ) = false;
	}

	for ( int i = 0; i < mouseButtonCount; ++i ) {
		mouseButtonState.at( i ) = false;
	}

	POINT cursorPos;
	GetCursorPos( &cursorPos );

	mousePos.x = cursorPos.x;
	mousePos.y = cursorPos.y;

	mousePrevPos = mousePos;

	mouseMove.x = 0;
	mouseMove.y = 0;

    limitMouseMove = true;
    maxMouseMove = int2( 50, 50 );

	awaitMouseMoveInterval = 20.0;

	lockedCursorPos.x = 100;		
	lockedCursorPos.y = 100;

	lockCursorPos = false;

}


InputManager::~InputManager() {}

void InputManager::updateMouseState( ) {
	Timer currTime;
	if ( Timer::lapse( currTime, awaitMouseMoveLastTick ) >= awaitMouseMoveInterval ) {
		POINT cursorPos;
		GetCursorPos( &cursorPos );

		mousePos.x = cursorPos.x;
		mousePos.y = cursorPos.y;

		if ( lockCursorPos ) {
			mouseMove = mousePos - lockedCursorPos;
			mousePrevPos = lockedCursorPos;

			SetCursorPos( lockedCursorPos.x, lockedCursorPos.y );
			mousePos = lockedCursorPos;
			mousePrevPos = lockedCursorPos;
		} else {
			mouseMove = mousePos - mousePrevPos;
			mousePrevPos = mousePos;
		}

        if (abs(mouseMove.x) > maxMouseMove.x)
            mouseMove.x = mouseMove.x > 0 ? maxMouseMove.x : -maxMouseMove.x;

        if (abs(mouseMove.y) > maxMouseMove.y)
            mouseMove.y = mouseMove.y > 0 ? maxMouseMove.y : -maxMouseMove.y;

		awaitMouseMoveLastTick.reset();
	}
}

void InputManager::onKeyboardButton( int key, bool pressed ) {
	if ( key < 0 || key >= keyboardKeyCount ) 
        return;

	keyboardButtonState.at( key ) = pressed;
}

void InputManager::onMouseButton( int key, bool pressed ) {
	if ( key < 0 || key >= mouseButtonCount ) 
        return;

	mouseButtonState.at( key ) = pressed;
}

void InputManager::lockCursor( bool lock ) {

    if (lock && !lockCursorPos) {
        POINT cursorPos;
	    GetCursorPos( &cursorPos );

        lockedCursorPos.x = cursorPos.x;		
	    lockedCursorPos.y = cursorPos.y;
    }

	lockCursorPos = lock;
}

bool InputManager::isCursorLocked() {
	return lockCursorPos;
}

bool InputManager::isKeyPressed( unsigned int key ) {
	if ( key >= keyboardKeyCount ) 
        throw std::exception( "InputManager::isKeyPressed - given key is outside of the accepted range." );

	return keyboardButtonState.at( key );
}

bool InputManager::isMouseButtonPressed( unsigned int button )
{
    if ( button >= mouseButtonCount ) 
        throw std::exception( "InputManager::isMouseButtonPressed - given key is outside of the accepted range." );

	return mouseButtonState.at( button );
}

int2 InputManager::getMouseMove() {
	return mouseMove;
}

int2 InputManager::getMousePos() {
     return mousePos;
}