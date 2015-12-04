#include "InputManager.h"

#include <windows.h>

using namespace Engine1;

InputManager::InputManager() {
	for ( int i = 0; i < keyboardKeyCount; ++i ) {
		keyboardButtonState.at( i ) = false;
	}

	for ( int i = 0; i < mouseKeyCount; ++i ) {
		mouseButtonState.at( i ) = false;
	}

	POINT cursorPos;
	GetCursorPos( &cursorPos );

	mousePos.x = cursorPos.x;
	mousePos.y = cursorPos.y;

	mousePrevPos = mousePos;

	mouseMove.x = 0;
	mouseMove.y = 0;

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

		awaitMouseMoveLastTick.reset();
	}
}

void InputManager::onKeyboardButton( int key, bool pressed ) {
	if ( 0 > key || keyboardKeyCount <= key ) return;

	keyboardButtonState.at( key ) = pressed;
}

void InputManager::onMouseButton( int key, bool pressed ) {
	if ( 0 > key || mouseKeyCount <= key ) return;

	mouseButtonState.at( key ) = pressed;
}

void InputManager::lockCursor( bool lock ) {
	lockCursorPos = lock;
}

bool InputManager::isCursorLocked() {
	return lockCursorPos;
}

bool InputManager::isKeyPressed( unsigned int key ) {
	if ( key >= keyboardKeyCount ) throw std::exception( "given key is outside of the accepted range" );

	return keyboardButtonState.at( key );
}

int2 InputManager::getMouseMove() {
	return mouseMove;
}