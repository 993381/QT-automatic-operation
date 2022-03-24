#ifndef MESSAGE_H
#define MESSAGE_H



class Message {
public:
    Message(){

    }
    ~Message(){}
    enum MsgType { MsgControl, MsgJson, MsgText, MsgData };
    int proto;
    MsgType type;

};


#endif//MESSAGE_H
