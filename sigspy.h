#ifndef SIGSPY_H
#define SIGSPY_H
// #include "signalspycallbackset.h"
#include <QObject>

void signal_begin_callback(QObject *caller, int method_index, void **argv);
void signal_end_callback(QObject *caller, int method_index);
void slot_begin_callback(QObject *caller, int method_index, void **argv);
void slot_end_callback(QObject *caller, int method_index);

#endif//SIGSPY_H
