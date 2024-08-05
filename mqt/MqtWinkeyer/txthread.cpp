/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Winkeyer Control
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////
#include <QDebug>

#include "winkeyerControl.h"
#include "TxThread.h"

TxThread::TxThread(WinkeyerControl *winkeyerControl, QObject *parent)
    : QThread(parent), winkeyerControl(winkeyerControl)
{
    qDebug() << "TxThread created";

}


void TxThread::run()
{
    QEventLoop eventLoop;
    timer = new QTimer();
    connect(timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    timer->setInterval(3);  // 3 ms interval

    while (true)
    {
        QMutexLocker locker(&winkeyerControl->mutex);


        // Wait until there is data to send or termination signal
        while (winkeyerControl->txQueue.isEmpty() && !shouldTerminate)
        {
            winkeyerControl->txCondition.wait(&winkeyerControl->mutex);
        }



        // Break the loop if termination is requested
        if (shouldTerminate)
        {
            break;
        }

        // Process immediate commands first
        processImmediateCommands();

        // Now that the condition is met, dequeue the data
        if (!winkeyerControl->txQueue.isEmpty())
        {
            QByteArray data = winkeyerControl->txQueue.dequeue();
            locker.unlock();

            // Write data immediately
            emit writeData(data);

            // Optional pacing delay for next iteration
            timer->start();
            eventLoop.exec(); // This will block until timer timeout
        }

    }

    delete timer;
}

void TxThread::stop()
{
    QMutexLocker locker(&winkeyerControl->mutex);
    shouldTerminate = true;
    winkeyerControl->txCondition.wakeAll();
}

void TxThread::writeABuffer(const QByteArray &data)
{
    emit writeData(data);
}

void TxThread::processImmediateCommands()
{
    if (winkeyerControl->breakinClear)
    {
        winkeyerControl->breakinClear = false;
        // Clear buffers and reset state...
    }
    if (winkeyerControl->newClear)
    {
        winkeyerControl->newClear = false;
        QByteArray cmd;
        cmd.append(CLRBUF_CMD);
        winkeyerControl->txQueue.prepend(cmd);
    }

    // Check and process other immediate commands ...

    if (winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getSpeed() != winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getSpeed())
    {
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->setSpeed( winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getSpeed());
        QByteArray cmd;
        cmd.append(SPEED_CMD);
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getSpeed());
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getMinwpm() != winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getMinwpm() ||
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getWpmrange() != winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getWpmrange())
    {
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->setMinwpm(winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getMinwpm());
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->setWpmrange(winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getWpmrange());
        QByteArray cmd;
        cmd.append(WPMRANGE_CMD);
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getMinwpm());
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getWpmrange());
        cmd.append(CMD_END);
        winkeyerControl->txQueue.prepend(cmd);

    }
    if (winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getStconst() != winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getStconst())
    {
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->setStconst(winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getStconst());
        QByteArray cmd;
        cmd.append(FREQ_CMD);
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getStconst());
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getWeight() != winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getWeight())
    {
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->setWeight(winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getWeight());
        QByteArray cmd;
        cmd.append(WEIGHT_CMD);
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getWeight());
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getLeadin() != winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getLeadin() ||
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getTail() != winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getTail())
    {
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->setLeadin(winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getLeadin());
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->setTail(winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getTail());
        QByteArray cmd;
        cmd.append(LEADTAIL_CMD);
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getLeadin());
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getTail());
        winkeyerControl->txQueue.prepend(cmd);

    }
    if (winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getXtnd() != winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getXtnd())
    {
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->setXtnd(winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getXtnd());
        QByteArray cmd;
        cmd.append(XTND_CMD);
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getXtnd());
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getKcomp() != winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getKcomp())
    {
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->setKcomp(winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getKcomp());
        QByteArray cmd;
        cmd.append(KCOMP_CMD);
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getKcomp());
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getFarns() != winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getFarns())
    {
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->setFarns(winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getFarns());
        QByteArray cmd;
        cmd.append(SETFARNS_CMD);
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getFarns());
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getSampadj() != winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getSampadj())
    {
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->setSampadj(winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getSampadj());
        QByteArray cmd;
        cmd.append(SAMPADJ_CMD);
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getSampadj());
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getDitdahratio() != winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getDitdahratio())
    {
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->setDitdahratio(winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getDitdahratio());
        QByteArray cmd;
        cmd.append(DUTY_CMD);
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getDitdahratio());
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getModereg() != winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getModereg())
    {
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->setModereg(winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getModereg());
        QByteArray cmd;
        cmd.append(MODE_CMD);
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getModereg());
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getX1mode() != winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getX1mode())
    {
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->setX1mode(winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getX1mode());
        QByteArray cmd;
        cmd.append(ADMIN_CMD);
        cmd.append(ADMIN_LDX1MODE);
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getX1mode());
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getPincfg() != winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getPincfg())
    {
        winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->setPincfg(winkeyerControl->getNewWinkeyStateStoragePtr()->getWkState()->getPincfg());
        QByteArray cmd;
        cmd.append(PINCFG_CMD);
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getPincfg());
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getcurTune() != winkeyerControl->getNewTune())
    {
        winkeyerControl->setcurTune(winkeyerControl->getcurTune());
        QByteArray cmd;
        cmd.append(KEYIMM_CMD);
        cmd.append(winkeyerControl->getcurTune() ? 1 :0);
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getDoGetPot())
    {
        winkeyerControl->setDoGetPot(false);
        QByteArray cmd;
        cmd.append(GETPOT_CMD);
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getCurPause() != winkeyerControl->getNewPause())
    {
        winkeyerControl->setCurPause(winkeyerControl->getNewPause());
        QByteArray cmd;
        cmd.append(PAUSE_CMD ? 1 : 0);
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getCurDirect() != winkeyerControl->getNewDirect())
    {
        winkeyerControl->setCurDirect(winkeyerControl->getNewDirect());
        QByteArray cmd;
        cmd.append(DIRECTKEY_CMD);
        cmd.append(winkeyerControl->getCurDirect());
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getDumpState())
    {
        winkeyerControl->setDumpState(false);
        QByteArray cmd;
        cmd.append(ADMIN_CMD);
        cmd.append(ADMIN_STATE);
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getNewX2mode())
    {
        winkeyerControl->setNewX2mode(false);
        QByteArray cmd;
        cmd.append(ADMIN_CMD);
        cmd.append(ADMIN_LDX2MODE);
        cmd.append(winkeyerControl->getX2modeValue());
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getNewVolume())
    {
        winkeyerControl->setNewVolume(false);
        QByteArray cmd;
        cmd.append(ADMIN_CMD);
        cmd.append(ADMIN_SETVOL);
        cmd.append(winkeyerControl->getVolumeValue());
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getNewRTTY())
    {
        winkeyerControl->setNewRTTY(false);
        QByteArray cmd;
        cmd.append(ADMIN_CMD);
        cmd.append(ADMIN_SETRTTY);
        cmd.append(winkeyerControl->getRttyValue1());
        cmd.append(winkeyerControl->getRttyValue2());
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getDumpDebug())
    {
        winkeyerControl->setDumpDebug(false);
        QByteArray cmd;
        cmd.append(ADMIN_CMD);
        cmd.append(ADMIN_DEBUG);
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getDumpPdl())
    {
        winkeyerControl->setDumpPdl(false);
        QByteArray cmd;
        cmd.append(ADMIN_CMD);
        cmd.append(ADMIN_A2DPDL);
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getDumpPot())
    {
        winkeyerControl->setDumpPot(false);
        QByteArray cmd;
        cmd.append(ADMIN_CMD);
        cmd.append(ADMIN_A2DPOT);
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getDoBlock())
    {
        winkeyerControl->setDoBlock(false);
        QByteArray cmd;
        cmd.append(DFLTS_CMD);
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getModereg());
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getSpeed());
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getStconst());
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getWeight());
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getLeadin());
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getTail());
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getMinwpm());
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getWpmrange());
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getXtnd());
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getKcomp());
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getFarns());
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getSampadj());
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getDitdahratio());
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getPincfg());
        cmd.append(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getWkState()->getX1mode());
        cmd.append(GETPOT_CMD);
        cmd.append(GETSTAT_CMD);
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getDoSoftReset())
    {
        winkeyerControl->setDoSoftReset(false);
        QByteArray cmd;
        cmd.append(ADMIN_CMD);
        cmd.append(ADMIN_RESET);
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getDoBackSpace())
    {
        winkeyerControl->setDoBackSpace(false);
        handleBackspace();
    }
    if (winkeyerControl->getSetHscwFlag())
    {
        winkeyerControl->setSetHscwFlag(false);
        QByteArray cmd;
        cmd.append(HSCW_CMD);
        cmd.append(winkeyerControl->getHscwValue() );
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getWK1Flag())
    {
        winkeyerControl->setWK1Flag(false);
        QByteArray cmd;
        cmd.append(HSCW_CMD);
        cmd.append(ADMIN_SETWK1);
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getWK2Flag())
    {
        winkeyerControl->setWK2Flag(false);
        QByteArray cmd;
        cmd.append(HSCW_CMD);
        cmd.append(ADMIN_SETWK2);
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getWK3Flag())
    {
        winkeyerControl->setWK3Flag(false);
        QByteArray cmd;
        cmd.append(HSCW_CMD);
        cmd.append(ADMIN_SETWK3);
        winkeyerControl->txQueue.prepend(cmd);
    }
    if (winkeyerControl->getCloseTxFlag())
    {
        winkeyerControl->setCloseTXFlag(false);
        QByteArray cmd;
        cmd.append(ADMIN_CMD);
        cmd.append(ADMIN_CLOSE);
        winkeyerControl->txQueue.prepend(cmd);
    }



}


void TxThread::handleBackspace()
{
    QQueue<QByteArray> &queue = winkeyerControl->txQueue;

    if (queue.isEmpty())
    {
        QByteArray cmd;
        cmd.append(BACKSP_CMD);
        queue.prepend(cmd);
    } else
    {
        QByteArray &lastItem = queue.head();
        if (!lastItem.isEmpty()) {
            lastItem.chop(1);  // Remove the last byte
            if (lastItem.isEmpty()) {
                queue.dequeue();  // Remove the item if it's empty
            }
        }
    }
}
