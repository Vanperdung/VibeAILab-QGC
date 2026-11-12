#pragma once

#include "UnitTest.h"

/// Tests the side states and the run end against the "[cal] " status texts of the FC.
class VibeAILabAccelCalibrationTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _sidesFollowTexts();
    void _poseText_data();
    void _poseText();
    void _endText_data();
    void _endText();
    void _ignoredText_data();
    void _ignoredText();
    void _startedTextStartsRunBeforeReply();
    void _startedTextAfterNoResponse();
    void _startAgainResetsSides();
    void _failedStartAgainKeepsRun_data();
    void _failedStartAgainKeepsRun();
    void _runEndsWhileStartAgainOnItsWay();
    void _textsIgnoredWhenIdle();
};
