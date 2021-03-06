//
//  Sound.h
//  hifi
//
//  Created by Stephen Birarda on 1/2/2014.
//  Modified by Athanasios Gaitatzes to add WAVE file support.
//  Copyright (c) 2014 HighFidelity, Inc. All rights reserved.
//

#ifndef __hifi__Sound__
#define __hifi__Sound__

#include <QtCore/QObject>

class QNetworkReply;

class Sound : public QObject {
    Q_OBJECT
public:
    Sound(const QUrl& sampleURL, QObject* parent = NULL);
    Sound(float volume, float frequency, float duration, float decay, QObject* parent = NULL);
    
    const QByteArray& getByteArray() { return _byteArray; }

private:
    QByteArray _byteArray;

    void downSample(const QByteArray& rawAudioByteArray);
    void interpretAsWav(const QByteArray& inputAudioByteArray, QByteArray& outputAudioByteArray);

private slots:
    void replyFinished(QNetworkReply* reply);
};

#endif /* defined(__hifi__Sound__) */
