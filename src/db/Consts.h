#ifndef DB_CONSTS_H
#define DB_CONSTS_H

#define COL_COUNT       11
#define COL_ID          0
#define COL_SUMMARY     1
#define COL_EXTRA       2
#define COL_CATEGORY    3
#define COL_SEVERITY    4
#define COL_PRIORITY    5
#define COL_REPEAT      6
#define COL_STATUS      7
#define COL_SOLUTION    8
#define COL_CREATED     9
#define COL_UPDATED     10

#define TABLE_BUGS      "Issue"
#define TABLE_HISTORY   "History"
#define TABLE_CATEGORY  "Category"
#define TABLE_SEVERITY  "Severity"
#define TABLE_PRIORITY  "Priority"
#define TABLE_STATUS    "Status"
#define TABLE_REPEAT    "Repeatability"
#define TABLE_HISTORY   "History"
#define TABLE_SOLUTION  "Solution"
#define TABLE_SETTINGS  "Settings"
#define TABLE_RELATIONS "Relations"

#define DICT_ID         "Id"
#define DICT_TITLE      "Title"
#define DICT_COL_ID     0
#define DICT_COL_TITLE  1

#define CATEGORY_NONE   0

#define STATUS_OPENED   100
#define STATUS_SOLVED   200
#define STATUS_CLOSED   300

#define PRIORITY_MIN    100
#define PRIORITY_LOW    200
#define PRIORITY_NORMAL 300
#define PRIORITY_HIGH   400
#define PRIORITY_URGENT 500

#define SEVERITY_TODO       100
#define SEVERITY_ENHANCE    200
#define SEVERITY_TEXT       300
#define SEVERITY_TRIVIAL    400
#define SEVERITY_ERROR      500
#define SEVERITY_BLUNDER    600
#define SEVERITY_CRUSH      700
#define SEVERITY_BLOCKER    800

#define SOLUTION_NONE       0
#define SOLUTION_FIXED      100
#define SOLUTION_UNREPEAT   200
#define SOLUTION_IRRECOVER  300
#define SOLUTION_DUPLICATE  400
#define SOLUTION_REJECTED   500
#define SOLUTION_SUSPENDED  600
#define SOLUTION_ABANDONED  700

#endif // DB_CONSTS_H