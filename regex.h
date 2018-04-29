#ifndef REGEX_INCL_H
#define REGEX_INCL_H

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#define MAXLEN 1024

#define IS_MATCH 0
#define NOT_MATCH 1

#define GENDER_EVAL 0
#define GENDER_UNION 1
#define GENDER_CONCAT 2
#define GENDER_MULTIP 3
#define GENDER_UNBRAC 4

#define REG_FULL_STR 0
#define REG_PART_STR 1

struct MatchCollection{

	struct Match *Matches;
	char *Pattern;

	int Gender;
	int MultipMin;
	int MultipMax;

	struct MatchCollection *Next;
	struct MatchCollection *Child;

};

struct Match{
	int FirstChar;
	int NextChar;
	struct Match *Next;
	struct SubMatch *SubMatches;
};

struct SubMatch{
	int FirstChar;
	int NextChar;
	struct SubMatch *Next;
};

//Regex engine main functions
void RegParse(struct MatchCollection *Adult);
void RegEval(struct MatchCollection *Eval, char *TestString);
int RegParseSplit(struct MatchCollection *Adult,int SplitPosn,int SplitOffs);
struct Match *AddMatch(struct MatchCollection *MatchCol,int StartPos,int EndPos,struct Match *LeftMatch,struct Match *RightMatch);
void AddSubMatch(struct Match *Parent,int StartPos,int EndPos);

//External use functions
int SimpleMatch(char *Pattern,char *TestString,int FullString);

//Test and debug functions
void RegDebug(char *Pattern,char *Test);
void RegTest(char *Pattern,char *TestString);

//Memory functions
struct MatchCollection *NewMatchCollection(char *Pattern);
struct Match *NewMatch(int StartPos,int EndPos);
void DisposeMatch(struct Match *Match);
void FreeMatchCollection(struct MatchCollection *MatchColLpr);

//Printing
void PrintSubMatch(struct SubMatch *SubMatch,char *TestString);
void PrintMatch(struct Match *Match,char *TestString);
void PrintMatches(struct MatchCollection *MatchColLpr,char *TestString);
void PrintDebug(struct MatchCollection *MatchLpr);


//Support Functions
void MatchRestart(struct MatchCollection *MatchTarget);
int CycleLastGroup(char *Pattern,int CharLpr);
int IsMultipPattern(char *Pattern,int CharLpr);
int IsMultip(char Input);
char *BlankString(int Size);

#endif //REGEX_INCL_H