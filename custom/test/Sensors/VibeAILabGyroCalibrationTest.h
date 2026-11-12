#pragma once

#include "UnitTest.h"

/// Tests the run states shared by all calibrations (start, cancel, command replies) and the gyro status texts.
class VibeAILabGyroCalibrationTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _startReply_data();
    void _startReply();
    void _startedTextStartsRunBeforeReply();
    void _startedTextAfterRefusedStart_data();
    void _startedTextAfterRefusedStart();
    void _endText_data();
    void _endText();
    void _cancelReply_data();
    void _cancelReply();
    void _requestsRefusedInWrongState();
    void _replyAfterRunEnded();
    void _replyToOtherRequestIgnored();
    void _textsIgnoredWhenIdle();
    void _clearResult();
};
