#pragma once

// app 向 gtm 索要txid
const int kGetTxid = 101;

// gtm 返回给 app 的消息，包括txid和需要的dbs列表
const int kTxid = 103;

// app 发送给 es
const int kStartTran = 102; // 开启一个事务
const int kAdd = 104;
const int kChange = 105;
const int kDelete = 106;
const int kSearch = 107;
const int kCommit = 108; // 提交一个事务

// es 发送给dbs
const int kData = 109;

const int kJudgeConflit = 2;

const int kLsn = 3;
const int kData = 4;

const int kTranSuccess = 5;
const int kLog = 6;
const int kLogAck = 7;
const int kTransAck = 8;