/** @file */

/**
 * \defgroup NationalInstruments National Instruments
 * @brief National Instruments related classes.
 * \ingroup Hardware
 */

#ifndef NIABSTRACTTASK_H
#define NIABSTRACTTASK_H

/**
 * \brief Wraps the call to the given function around some logic for error
 * checking
 *
 * If an error has occurred during the function call, NIAbstractTask::onError()
 * is called.
 *
 * \param functionCall The function to be called
 */
#ifdef WITH_HARDWARE
#define DAQmxErrChk(functionCall) { \
        if (!task) { \
            throw std::runtime_error("Task not initialized"); \
        } \
        if (DAQmxFailed(functionCall)) { \
            onError(); \
        } \
}
#else
#define DAQmxErrChk(functionCall) {}
#endif

#include <QObject>

#include "natinst.h"

/**
 * @brief The NIAbstractTask class is an abstract wrapper for a DAQmx Task.
 *
 * \ingroup NationalInstruments
 */

class NIAbstractTask : public QObject
{
    Q_OBJECT
public:
    explicit NIAbstractTask(QObject *parent = nullptr);
    virtual ~NIAbstractTask();

    bool isInitialized();
    bool isTaskDone();

    QString getTaskName() const;
    void setTaskName(const QString &value);

    void appendToTaskName(const QString &suffix);
    void configureSampleClockTiming(const QString &source,
                                    NI::int32 activeEdge,
                                    NI::int32 sampleMode);

    double getSampleRate() const;
    void setSampleRate(double value);

    NI::uInt64 getNSamples() const;
    void setNSamples(const NI::uInt64 &value);

signals:
    void error();

public slots:
    void initializeTask();
    void start();
    void stop();
    void clear();

protected:
    [[ noreturn ]] void onError() const;
#ifdef NIDAQMX_HEADERS
    NI::TaskHandle task = nullptr;
#endif
    NI::uInt64 nSamples;

private:
    virtual void initializeTask_impl() = 0;

    char *errBuff;
    QString taskName;
    double sampleRate;
};

#endif // NIABSTRACTTASK_H
