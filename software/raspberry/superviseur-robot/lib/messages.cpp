/*
 * Copyright (C) 2018 dimercur
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "messages.h"
#include <exception>
#include <stdexcept>
#include <string>

/*
 * Constants used with ToString method, for printing message id
 */

const string MESSAGE_ID_STRING[] = {
    "Empty",
    "Log",
    "Answer",
    "Open serial com",
    "Close serial com",
    "Open camera",
    "Close camera",
    "Ask for arena",
    "Arena confirmed",
    "Arena infirmed",
    "Compute position",
    "Stop compute position",
    "Position,
    "Image",
    "Robot ping",
    "Robot reset",
    "Robot start with wtachdog",
    "Robot start without wtachdog",
    "Robot reload watchdog",
    "Robot move",
    "Robot turn",
    "Robot go forward",
    "Robot go backward",
    "Robot go left",
    "Robot go right",
    "Robot stop",
    "Robot poweroff",
    "Robot get battery",
    "Robot battery level",
    "Robot get state",
    "Robot current state"

};

/**
 * Create a new, empty message
 */
Message::Message() {
    this->messageID = MESSAGE_EMPTY;
}

/**
 * Destroy message
 */
Message::~Message() {
}

/**
 * Translate content of message into a string that can be displayed
 * @return A string describing message contents
 */
string Message::ToString() {
    if (CheckID(this->messageID))
        return "Id: \"" + MESSAGE_ID_STRING[this->messageID] + "\"";
    else
        return "Invalid message";
}

/**
 * Allocate a new mesage and copy contents of current message
 * @return A message, copy of current
 */
Message* Message::Copy() {
    Message *msg = new Message();

    return msg;
}

/**
 * Get message ID
 * @return Current message ID
 */
bool Message::CheckID(MessageID id) {
    if ((id != MESSAGE_EMPTY) &&
            (id != MESSAGE_ARENA_CONFIRM) &&
            (id != MESSAGE_ARENA_INFIRM) &&
            (id != MESSAGE_ASK_ARENA) &&
            (id != MESSAGE_CAM_CLOSE) &&
            (id != MESSAGE_CAM_OPEN) &&
            (id != MESSAGE_CLOSE_COM) &&
            (id != MESSAGE_COMPUTE_POSITION) &&
            (id != MESSAGE_OPEN_COM) &&
            (id != MESSAGE_ROBOT_GET_BATTERY) &&
            (id != MESSAGE_ROBOT_GET_STATE) &&
            (id != MESSAGE_ROBOT_GO_BACK) &&
            (id != MESSAGE_ROBOT_GO_FORWARD) &&
            (id != MESSAGE_ROBOT_GO_LEFT) &&
            (id != MESSAGE_ROBOT_GO_RIGHT) &&
            (id != MESSAGE_ROBOT_PING) &&
            (id != MESSAGE_ROBOT_POWEROFF) &&
            (id != MESSAGE_ROBOT_RELOAD_WD) &&
            (id != MESSAGE_ROBOT_RESET) &&
            (id != MESSAGE_ROBOT_START_WITHOUT_WD) &&
            (id != MESSAGE_ROBOT_START_WITH_WD) &&
            (id != MESSAGE_ROBOT_STOP) &&
            (id != MESSAGE_STOP_COMPUTE_POSITION)) {
        return false;
    } else return true;
}

/* MessageFloat */

/**
 * Create a new, empty float message
 */
MessageFloat::MessageFloat() {
    value = 0.0;
}

/**
 * Create a new float message, with given ID and value
 * @param id Message ID
 * @param val Message value
 * @throw std::runtime_error if message ID is incompatible with float data
 */
MessageFloat::MessageFloat(MessageID id, float val) {
    MessageFloat::SetID(id);

    value = val;
}

/**
 * Set message ID
 * @param id Message ID
 * @throw std::runtime_error if message ID is incompatible with float data
 */
void MessageFloat::SetID(MessageID id) {
    if (CheckID(id))
        messageID = id;
    else
        throw std::runtime_error {
        "Invalid message id for MessageFloat"
    };
}

/**
 * Translate content of message into a string that can be displayed
 * @return A string describing message contents
 */
string MessageFloat::ToString() {
    if (CheckID(this->messageID))
        return "Id: \"" + MESSAGE_ID_STRING[this->messageID] + "\"\nValue: " + to_string(this->value);
    else
        return "Invalid message";
}

/**
 * Allocate a new mesage and copy contents of current message
 * @return A message, copy of current
 */
Message* MessageFloat::Copy() {
    return new MessageFloat(this->messageID, this->value);
}

/**
 * Verify if message ID is compatible with current message type
 * @param id Message ID
 * @return true, if message ID is acceptable, false otherwise
 */
bool MessageFloat::CheckID(MessageID id) {
    if ((id != MESSAGE_ANGLE_POSITION) &&
            (id != MESSAGE_ANGULAR_SPEED) &&
            (id != MESSAGE_BATTERY) &&
            (id != MESSAGE_BETA) &&
            (id != MESSAGE_LINEAR_SPEED) &&
            (id != MESSAGE_TORQUE)) {
        return false;
    } else return true;
}

/* class MessageString */

/**
 * Create a new, empty string message
 */
MessageString::MessageString() {
    s = string("");
}

/**
 * Create a new string message, with given ID and string
 * @param id Message ID
 * @param s Message string
 * @throw std::runtime_error if message ID is incompatible with string data
 */
MessageString::MessageString(MessageID id, string s) {
    MessageString::SetID(id);

    this->s = s;
}

/**
 * Set message ID
 * @param id Message ID
 * @throw std::runtime_error if message ID is incompatible with string data
 */
void MessageString::SetID(MessageID id) {
    if (CheckID(id))
        messageID = id;
    else
        throw std::runtime_error {
        "Invalid message id for MessageString"
    };
}

/**
 * Translate content of message into a string that can be displayed
 * @return A string describing message contents
 */
string MessageString::ToString() {
    if (CheckID(this->messageID))
        return "Id: \"" + MESSAGE_ID_STRING[this->messageID] + "\"\nString: \"" + this->s + "\"";
    else
        return "Invalid message";
}

/**
 * Allocate a new message and copy contents of current message
 * @return A message, copy of current
 */
Message* MessageString::Copy() {
    return new MessageString(this->messageID, this->s);
}

/**
 * Verify if message ID is compatible with current message type
 * @param id Message ID
 * @return true, if message ID is acceptable, false otherwise
 */
bool MessageString::CheckID(MessageID id) {
    if ((id != MESSAGE_LOG)) {
        return false;
    } else return true;
}

/* class MessageBool */

/**
 * Create a new, empty boolean message
 */
MessageBool::MessageBool() {
    state = false;
}

/**
 * Create a new boolean message, with given ID and boolean value
 * @param id Message ID
 * @param state Boolean value
 * @throw std::runtime_error if message ID is incompatible with boolean data
 */
MessageBool::MessageBool(MessageID id, bool state) {
    MessageBool::SetID(id);

    this->state = state;
}

/**
 * Set message ID
 * @param id Message ID
 * @throw std::runtime_error if message ID is incompatible with boolean data
 */
void MessageBool::SetID(MessageID id) {
    if (CheckID(id))
        messageID = id;
    else
        throw std::runtime_error {
        "Invalid message id for MessageBool"
    };
}

/**
 * Translate content of message into a string that can be displayed
 * @return A string describing message contents
 */
string MessageBool::ToString() {
    if (CheckID(this->messageID))
        return "Id: \"" + MESSAGE_ID_STRING[this->messageID] + "\"\nState: \"" + to_string(this->state) + "\"";
    else
        return "Invalid message";
}

/**
 * Allocate a new message and copy contents of current message
 * @return A message, copy of current
 */
Message* MessageBool::Copy() {
    return new MessageBool(this->messageID, this->state);
}

/**
 * Verify if message ID is compatible with current message type
 * @param id Message ID
 * @return true, if message ID is acceptable, false otherwise
 */
bool MessageBool::CheckID(MessageID id) {
    if ((id != MESSAGE_EMERGENCY_STOP) &&
            (id != MESSAGE_USER_PRESENCE)) {
        return false;
    } else return true;
}
