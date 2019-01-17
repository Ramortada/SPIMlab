#include <iostream>
#include "galvoramp.h"

using namespace NI;

#define CHANNEL_NAME "galvoRampAOChan"

GalvoRamp::GalvoRamp(QObject *parent) : NIAbstractTask(parent)
{
}

GalvoRamp::~GalvoRamp()
{
    clear();
}

void GalvoRamp::setPhysicalChannel(QString channel)
{
    physicalChannel = channel;
    clear();
}

void GalvoRamp::setTriggerSource(QString source)
{
    triggerSource = source;
    clear();
}

void GalvoRamp::setCameraParams(int nSamples, int nRamp, double rate)
{
    this->nSamples = nSamples;
    this->nRamp = nRamp;
    this->rate = rate;

    if (isInitialized()) {
        computeWaveform();
        configureTiming();
        write();
    }
}

void GalvoRamp::setupWaveform(double offset, double amplitude, int delay)
{
    this->offset = offset;
    this->amplitude = amplitude;
    this->delay = delay;

    if (isInitialized()) {
        computeWaveform();
        write();
    }
}

void GalvoRamp::initializeTask_impl()
{
#ifdef WITH_HARDWARE
    DAQmxErrChk(DAQmxCreateTask("galvoRampAO", &task));

    DAQmxErrChk(
        DAQmxCreateAOVoltageChan(
            task,
            physicalChannel.toLatin1(),
            CHANNEL_NAME,
            -10.0,  // minVal
            10.0,  // maxVal
            DAQmx_Val_Volts,  // units
            nullptr // customScaleName
            )
        );
    DAQmxErrChk(
        DAQmxCfgDigEdgeStartTrig(
            task,
            triggerSource.toLatin1(),
            DAQmx_Val_Rising
            )
        );
    DAQmxErrChk(DAQmxSetStartTrigRetriggerable(task, true));

    computeWaveform();
    configureTiming();
    write();
#endif
}

void GalvoRamp::configureTiming()
{
#ifdef WITH_HARDWARE
    DAQmxErrChk(
        DAQmxCfgSampClkTiming(
            task,
            nullptr,  // clock source (onboard if NULL)
            rate,  // samples per second per channel
            DAQmx_Val_Rising,
            DAQmx_Val_FiniteSamps,
            static_cast<uInt64>(nSamples)
            )
        );
#endif
}

void GalvoRamp::write()
{
#ifdef WITH_HARDWARE
    DAQmxErrChk(
        DAQmxWriteAnalogF64(
            task,
            waveform.size(),
            false,  // autostart
            10,  // timeout in seconds
            DAQmx_Val_GroupByChannel,
            waveform.data(),
            nullptr,
            nullptr  // reserved
            )
        );
#endif
}

void GalvoRamp::computeWaveform()
{
    QVector<double> temp(nSamples, 0);
    double halfAmplitude = 0.5 * amplitude;
    int i = 0;
    for (; i < nRamp; ++i)
        temp[i] = offset - halfAmplitude + amplitude * i / nRamp;
    int nRamp2 = nSamples - nRamp;
    for (; i < nSamples; ++i)
        temp[i] = offset + halfAmplitude - amplitude * (i - nRamp) / nRamp2;

    if (delay == 0) {
        waveform = temp;
        return;
    }

    i = 0;
    waveform.clear();
    waveform.reserve(nSamples);

    if (delay > 0) {
        waveform << temp.mid(nSamples - delay - 1, delay);
        waveform << temp.mid(0, nSamples - delay);
    } else {
        delay = -delay;
        waveform << temp.mid(delay, nSamples - delay);
        waveform << temp.mid(0, delay);
    }
}
