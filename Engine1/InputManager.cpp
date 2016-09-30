#include "InputManager.h"

#include <windows.h>

using namespace Engine1;

InputManager::InputManager() {
	for ( int i = 0; i < keyboardKeyCount; ++i ) {
		m_keyboardButtonState.at( i ) = false;
	}

	for ( int i = 0; i < mouseButtonCount; ++i ) {
		m_mouseButtonState.at( i ) = false;
	}

	POINT cursorPos;
	GetCursorPos( &cursorPos );

	m_mousePos.x = cursorPos.x;
	m_mousePos.y = cursorPos.y;

	m_mousePrevPos = m_mousePos;

	m_mouseMove.x = 0;
	m_mouseMove.y = 0;

    m_limitMouseMove = true;
    m_maxMouseMove = int2( 50, 50 );

	m_awaitMouseMoveInterval = 20.0;

	m_lockedCursorPos.x = 100;		
	m_lockedCursorPos.y = 100;

	m_lockCursorPos = false;

}


InputManager::~InputManager() {}

void InputManager::updateMouseState( ) {
	Timer currTime;
	if ( Timer::getElapsedTime( currTime, m_awaitMouseMoveLastTick ) >= m_awaitMouseMoveInterval ) {
		POINT cursorPos;
		GetCursorPos( &cursorPos );

		m_mousePos.x = cursorPos.x;
		m_mousePos.y = cursorPos.y;

		if ( m_lockCursorPos ) {
			m_mouseMove = m_mousePos - m_lockedCursorPos;
			m_mousePrevPos = m_lockedCursorPos;

			SetCursorPos( m_lockedCursorPos.x, m_lockedCursorPos.y );
			m_mousePos = m_lockedCursorPos;
			m_mousePrevPos = m_lockedCursorPos;
		} else {
			m_mouseMove = m_mousePos - m_mousePrevPos;
			m_mousePrevPos = m_mousePos;
		}

        if (abs(m_mouseMove.x) > m_maxMouseMove.x)
            m_mouseMove.x = m_mouseMove.x > 0 ? m_maxMouseMove.x : -m_maxMouseMove.x;

        if (abs(m_mouseMove.y) > m_maxMouseMove.y)
            m_mouseMove.y = m_mouseMove.y > 0 ? m_maxMouseMove.y : -m_maxMouseMove.y;

		m_awaitMouseMoveLastTick.reset();
	}
}

void InputManager::onKeyboardButton( int key, bool pressed ) {
	if ( key < 0 || key >= keyboardKeyCount ) 
        return;

	m_keyboardButtonState.at( key ) = pressed;
}

void InputManager::onMouseButton( int key, bool pressed ) {
	if ( key < 0 || key >= mouseButtonCount ) 
        return;

	m_mouseButtonState.at( key ) = pressed;
}

void InputManager::lockCursor( bool lock ) {

    if (lock && !m_lockCursorPos) {
        POINT cursorPos;
	    GetCursorPos( &cursorPos );

        m_lockedCursorPos.x = cursorPos.x;		
	    m_lockedCursorPos.y = cursorPos.y;
    }

	m_lockCursorPos = lock;
}

bool InputManager::isCursorLocked() {
	return m_lockCursorPos;
}

bool InputManager::isKeyPressed( unsigned int key ) {
	if ( key >= keyboardKeyCount ) 
        throw std::exception( "InputManager::isKeyPressed - given key is outside of the accepted range." );

	return m_keyboardButtonState.at( key );
}

bool InputManager::isMouseButtonPressed( unsigned int button )
{
    if ( button >= mouseButtonCount ) 
        throw std::exception( "InputManager::isMouseButtonPressed - given key is outside of the accepted range." );

	return m_mouseButtonState.at( button );
}

int2 InputManager::getMouseMove() {
	return m_mouseMove;
}

int2 InputManager::getMousePos() {
     return m_mousePos;
}