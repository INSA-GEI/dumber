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
 * @brief Constants used with ToString method, for printing message id
 */

const string MESSAGE_ID_STRING[] = {
    "Empty",
    "Log",
    "Answer",
    "Monitor connection lost",
    "Open serial com",
    "Close serial com",
    "Open camera",
    "Close camera",
    "Ask for arena",
    "Arena confirmed",
    "Arena infirmed",
    "Compute position",
    "Stop compute position",
    "Position",
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

/*
 * @brief Constants used with ToString method, for printing answer id
 */
const string ANSWER_ID_STRING[] = {
    "Acknowledge",
    "Not Acknowledge",
    "Robot lost",
    "Timeout error",
    "Unknown command",
    "Invalid or refused command",
    "Checksum error"
};

/**
 * Create a new, empty message
 */
Message::Message() {
    this->messageID = MESSAGE_EMPTY;
}

/**
 * Create a new, empty message
 */
Message::Message(MessageID id) {
    SetID(id);
}

/**
 * Destroy message
 */
Message::~Message() {
}

/**
 * Set message ID
 * @param id Message ID
 */
void Message::SetID(MessageID id) {
    if (CheckID(id)) {
        this->messageID = id;
    } else throw std::runtime_error {"Invalid message id for Message"};
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
            (id != MESSAGE_MONITOR_LOST) &&
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

/* MessageInt */

/**
 * Create a new, empty int message
 */
MessageInt::MessageInt() {
    value = 0.0;
}

/**
 * Create a new int message, with given ID and value
 * @param id Message ID
 * @param val Message value
 * @throw std::runtime_error if message ID is incompatible with int data
 */
MessageInt::MessageInt(MessageID id, int val) {
    MessageInt::SetID(id);

    value = val;
}

/**
 * Set message ID
 * @param id Message ID
 * @throw std::runtime_error if message ID is incompatible with float data
 */
void MessageInt::SetID(MessageID id) {
    if (CheckID(id))
        messageID = id;
    else
        throw std::runtime_error {
        "Invalid message id for MessageInt"
    };
}

/**
 * Translate content of message into a string that can be displayed
 * @return A string describing message contents
 */
string MessageInt::ToString() {
    if (CheckID(this->messageID))
        return "Id: \"" + MESSAGE_ID_STRING[this->messageID] + "\"\nValue: " + to_string(this->value);
    else
        return "Invalid message";
}

/**
 * Allocate a new message and copy contents of current message
 * @return A message, copy of current
 */
Message* MessageInt::Copy() {
    return new MessageInt(this->messageID, this->value);
}

/**
 * Verify if message ID is compatible with current message type
 * @param id Message ID
 * @return true, if message ID is acceptable, false otherwise
 */
bool MessageInt::CheckID(MessageID id) {
    if ((id != MESSAGE_ROBOT_TURN) &&
            (id != MESSAGE_ROBOT_MOVE)) {
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

/* class MessageImg */

/**
 * Create a new, empty image message
 */
MessageImg::MessageImg() {
    image = NULL;
}

/**
 * Create a new image message, with given ID and image
 * @param id Message ID
 * @param image Image
 * @throw std::runtime_error if message ID is incompatible with image
 */
MessageImg::MessageImg(MessageID id, Img* image) {
    MessageImg::SetID(id);
    MessageImg::SetImage(image);
}

/**
 * Destroy Image message
 */
MessageImg::~MessageImg() {
    delete (this->image);
}

/**
 * Set message ID
 * @param id Message ID
 * @throw std::runtime_error if message ID is incompatible with image
 */
void MessageImg::SetID(MessageID id) {
    if (CheckID(id))
        messageID = id;
    else
        throw std::runtime_error {
        "Invalid message id for MessageImg"
    };
}

/**
 * Set message image
 * @param image Reference to image object
 */
void MessageImg::SetImage(Img* image) {
    this->image = image->Copy();
}

/**
 * Translate content of message into a string that can be displayed
 * @return A string describing message contents
 */
string MessageImg::ToString() {
    if (CheckID(this->messageID))
        return "Id: \"" + MESSAGE_ID_STRING[this->messageID] + "\"\n" + this->image->ToString();
    else
        return "Invalid message";
}

/**
 * Allocate a new message and copy contents of current message
 * @return A message, copy of current
 */
Message* MessageImg::Copy() {
    
    return new MessageImg(this->messageID, this->image->Copy());
}

/**
 * Verify if message ID is compatible with current message type
 * @param id Message ID
 * @return true, if message ID is acceptable, false otherwise
 */
bool MessageImg::CheckID(MessageID id) {
    if (id != MESSAGE_IMAGE) {
        return false;
    } else return true;
}

/* class MessageAnswer*/

/**
 * Create a new, empty answer message
 */
MessageAnswer::MessageAnswer() {
    answer=ANSWER_ACK;
}

/**
 * Create a new answer message, with given ID and answer
 * @param id Message ID
 * @param ans Answer ID
 * @throw std::runtime_error if message ID is incompatible with string data
 */
MessageAnswer::MessageAnswer(MessageID id, AnswerID ans) {
    MessageAnswer::SetID(id);
    MessageAnswer::SetAnswer(ans);
}

/**
 * Set message ID
 * @param id Message ID
 * @throw std::runtime_error if message ID is incompatible with answer message
 */
void MessageAnswer::SetID(MessageID id) {
    if (CheckID(id))
        messageID = id;
    else
        throw std::runtime_error {
        "Invalid message id for MessageAnswer"
    };
}

/**
 * Set message answer
 * @param ans Answer ID
 * @throw std::runtime_error if answer ID is incompatible with answer data
 */
void MessageAnswer::SetAnswer(AnswerID ans) {
    if ((ans != ANSWER_ACK) &&
            (ans != ANSWER_NACK) &&
            (ans != ANSWER_LOST_ROBOT) &&
            (ans != ANSWER_ROBOT_CHECKSUM) &&
            (ans != ANSWER_ROBOT_ERROR) &&
            (ans != ANSWER_ROBOT_TIMEOUT) &&
            (ans != ANSWER_ROBOT_UNKNOWN_COMMAND)) {
        this->answer = answer;
    } else {
        throw std::runtime_error{
            "Invalid answer for MessageAnswer"};
    }
}

/**
 * Translate content of message into a string that can be displayed
 * @return A string describing message contents
 */
string MessageAnswer::ToString() {
    if (CheckID(this->messageID))
        return "Id: \"" + MESSAGE_ID_STRING[this->messageID] + "\"\nAnswer: \"" + ANSWER_ID_STRING[this->answer] + "\"";
    else
        return "Invalid message";
}

/**
 * Allocate a new message and copy contents of current message
 * @return A message, copy of current
 */
Message* MessageAnswer::Copy() {
    return new MessageAnswer(this->messageID, this->answer);
}

/**
 * Verify if message ID is compatible with current message type
 * @param id Message ID
 * @return true, if message ID is acceptable, false otherwise
 */
bool MessageAnswer::CheckID(MessageID id) {
    if ((id != MESSAGE_ANSWER)) {
        return false;
    } else return true;
}

/* class MessageBattery */

/**
 * Create a new, empty battery message
 */
MessageBattery::MessageBattery() {
    this->level = BATTERY_UNKNOWN;
}

/**
 * Create a new battery message, with given ID and battery level
 * @param id Message ID
 * @param level Battery level
 * @throw std::runtime_error if message ID is incompatible with battery
 */
MessageBattery::MessageBattery(MessageID id, BatteryLevel level) {
    MessageBattery::SetID(id);
    MessageBattery::SetLevel(level);
}

/**
 * Set message ID
 * @param id Message ID
 * @throw std::runtime_error if message ID is incompatible with battery
 */
void MessageBattery::SetID(MessageID id) {
    if (CheckID(id))
        messageID = id;
    else
        throw std::runtime_error {
        "Invalid message id for MessageBattery"
    };
}

/**
 * Set battery level
 * @param level Battery level
 */
void MessageBattery::SetLevel(BatteryLevel level) {
    if ((level < BATTERY_UNKNOWN) || (level > BATTERY_FULL)) {
        throw std::runtime_error{
            "Invalid battery level for MessageBattery"};
    } else {
        this->level = level;
    }
}

/**
 * Translate content of message into a string that can be displayed
 * @return A string describing message contents
 */
string MessageBattery::ToString() {
    string levelString;
    
    switch (this->level) {
        case BATTERY_UNKNOWN:
            levelString="Unknown";
            break;
        case BATTERY_EMPTY:
            levelString="Empty";
            break;
        case BATTERY_LOW:
            levelString="Low";
            break;
        case BATTERY_FULL:
            levelString="Full";
            break;
        default:
            levelString="Invalid";
    }
    
    if (CheckID(this->messageID))
        return "Id: \"" + MESSAGE_ID_STRING[this->messageID] + "\"\nBattery level: \"" + levelString + "\"";
    else
        return "Invalid message";
}

/**
 * Allocate a new message and copy contents of current message
 * @return A message, copy of current
 */
Message* MessageBattery::Copy() {
    return new MessageBattery(this->messageID, this->level);
}

/**
 * Verify if message ID is compatible with current message type
 * @param id Message ID
 * @return true, if message ID is acceptable, false otherwise
 */
bool MessageBattery::CheckID(MessageID id) {
    if ((id != MESSAGE_ROBOT_BATTERY_LEVEL)) {
        return false;
    } else return true;
}

/* class MessagePosition */

/**
 * Create a new, empty string message
 */
MessagePosition::MessagePosition() {
    this->pos.angle = 0.0;
    this->pos.robotId = 0;
    this->pos.center.x=0.0;
    this->pos.center.y=0.0;
    this->pos.direction.x=0.0;
    this->pos.direction.y=0.0;
}

/**
 * Create a new string message, with given ID and string
 * @param id Message ID
 * @param s Message string
 * @throw std::runtime_error if message ID is incompatible with string data
 */
MessagePosition::MessagePosition(MessageID id, Position& pos) {
    MessagePosition::SetID(id);
    MessagePosition::SetPosition(pos);
}

/**
 * Set message ID
 * @param id Message ID
 * @throw std::runtime_error if message ID is incompatible with string data
 */
void MessagePosition::SetID(MessageID id) {
    if (CheckID(id))
        messageID = id;
    else
        throw std::runtime_error {
        "Invalid message id for MessagePosition"
    };
}

/**
 * Set position
 * @param pos Reference to position
 */
void MessagePosition::SetPosition(Position& pos) {
    this->pos.angle = pos.angle;
    this->pos.robotId = pos.robotId;
    this->pos.center = pos.center;
    this->pos.direction = pos.direction;
}
    
/**
 * Translate content of message into a string that can be displayed
 * @return A string describing message contents
 */
string MessagePosition::ToString() {
    if (CheckID(this->messageID))
        return "Id: \"" + MESSAGE_ID_STRING[this->messageID] + "\"\nPosition: \"" + to_string(this->pos.center.x) + ";" + to_string(this->pos.center.y) + "\"";
    else
        return "Invalid message";
}

/**
 * Allocate a new message and copy contents of current message
 * @return A message, copy of current
 */
Message* MessagePosition::Copy() {
    return new MessagePosition(this->messageID, this->pos);
}

/**
 * Verify if message ID is compatible with current message type
 * @param id Message ID
 * @return true, if message ID is acceptable, false otherwise
 */
bool MessagePosition::CheckID(MessageID id) {
    if ((id != MESSAGE_POSITION)) {
        return false;
    } else return true;
}


/* class MessageState */

/**
 * Create a new, empty state message
 */
MessageState::MessageState() {
    state = ROBOT_NOT_BUSY;
}

/**
 * Create a new string message, with given ID and string
 * @param id Message ID
 * @param s Message string
 * @throw std::runtime_error if message ID is incompatible with string data
 */
MessageState::MessageState(MessageID id, RobotState state) {
    MessageState::SetID(id);
    MessageState::SetState(state);
}

/**
 * Set message ID
 * @param id Message ID
 * @throw std::runtime_error if message ID is incompatible with robot state
 */
void MessageState::SetID(MessageID id) {
    if (CheckID(id))
        messageID = id;
    else
        throw std::runtime_error {
        "Invalid message id for MessageState"
    };
}

/**
 * Set robot state
 * @param state Robot state
 */
void MessageState::SetState(RobotState state) {
    if ((state != ROBOT_NOT_BUSY) && (state != ROBOT_BUSY)) {
        throw std::runtime_error{
            "Invalid state for MessageState"};
    } else {
        this->state = state;
    }
}
    
/**
 * Translate content of message into a string that can be displayed
 * @return A string describing message contents
 */
string MessageState::ToString() {
    string stateString;
    
    if (this->state == ROBOT_NOT_BUSY) stateString="Not busy";
    else if (this->state == ROBOT_BUSY) stateString="Busy";
    else stateString="Invalid state";
    
    if (CheckID(this->messageID))
        return "Id: \"" + MESSAGE_ID_STRING[this->messageID] + "\"\nState: \"" + stateString + "\"";
    else
        return "Invalid message";
}

/**
 * Allocate a new message and copy contents of current message
 * @return A message, copy of current
 */
Message* MessageState::Copy() {
    return new MessageState(this->messageID, this->state);
}

/**
 * Verify if message ID is compatible with current message type
 * @param id Message ID
 * @return true, if message ID is acceptable, false otherwise
 */
bool MessageState::CheckID(MessageID id) {
    if ((id != MESSAGE_ROBOT_CURRENT_STATE)) {
        return false;
    } else return true;
}