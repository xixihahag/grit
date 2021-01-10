#pragma once

// app 向 gtm 索要txid
const int kGetTxid = 101;

// app 通知 dbs 开启一个事务
const int kStartTran = 102;

const int kJudgeConflit = 2;

const int kLsn = 3;
const int kData = 4;

const int kTranSuccess = 5;
const int kLog = 6;
const int kLogAck = 7;
const int kTransAck = 8;