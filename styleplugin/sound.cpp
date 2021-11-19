#include "sound.h"

#include <QCoreApplication>
#include <QFile>
#include <QtMultimedia/QSound>
#include <QDebug>
#include <QStandardPaths>


void sound::playSound(QString wav)
{
    // probono: Play wav because it is lowest latency
    // QSound::play("://sounds/EmptyTrash.wav"); // FIXME: Cannot get this to play from a resource

    QString soundFile = nullptr;
    QStringList locationCandidates =  QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    foreach (QString locationCandidate, locationCandidates) {
        if(QFile::exists(locationCandidate + "/panda/sounds/" + wav)) {
            soundFile = locationCandidate + "/panda/sounds/" + wav;
        }
    }

    if(soundFile != nullptr) {
        QSound::play(soundFile);
    } else {
        qDebug() << "Sound not found";
    }

}
