// Copyright (c) 2018 Alex Rowley
// Distributed under the MIT software license
// http://www.opensource.org/licenses/mit-license.php.

#include "regex.h"

int Mode = 0;
char *TempPattern;
struct Match *MatchStack;


int SimpleMatch(char *Pattern,char *TestString,int FullString){

	struct MatchCollection *Parent;

	int TestStringMax;
	int Return;
	struct Match *Match;

	TestStringMax=0;
	if(FullString) while(TestString[TestStringMax]!='\0') TestStringMax++;

	TempPattern = BlankString(MAXLEN);

	Parent = NewMatchCollection(Pattern);

	RegParse(Parent);
	RegEval(Parent,TestString);

	Return = 0;
	if (FullString){
		Match = Parent->Matches;
		while(Match!=NULL){
			if(Match->FirstChar==0 && Match->NextChar==TestStringMax) Return = 1;
			Match=Match->Next;
		}
	}
	else if(Parent->Matches!=NULL) Return = 1;

	free(TempPattern);
	FreeMatchCollection(Parent);

	return Return;

}

void RegTest(char *Pattern,char *TestString){

	struct MatchCollection *Parent;


	printf("Testing RegEx Engine..\nPattern:\t\"%s\"\nTest string:\t\"%s\"\n",Pattern,TestString);

	TempPattern = BlankString(MAXLEN);

	Parent = NewMatchCollection(Pattern);

	RegParse(Parent);

	RegEval(Parent,TestString);
	
	PrintMatches(Parent,TestString);

	free(TempPattern);

	FreeMatchCollection(Parent);
	
}

void RegDebug(char *Pattern, char *TestString){

	struct MatchCollection *Parent;

	TempPattern = BlankString(MAXLEN);

	Parent = NewMatchCollection(Pattern);

	RegParse(Parent);

	RegEval(Parent,TestString);

	PrintDebug(Parent);

	FreeMatchCollection(Parent);
	free(TempPattern);
}

void RegParse(struct MatchCollection *Adult){

	int CharLpr;
	int SplitPosn;
	char *Pattern;
	int BracketLevel;

	Pattern = Adult->Pattern;
	CharLpr=0;
	BracketLevel=0;

	while(Pattern[CharLpr]!='\0'){

		TempPattern[CharLpr]=Pattern[CharLpr];
		
		switch(Pattern[CharLpr]){
			
			case '+':
				
				if(BracketLevel==0){
					//There is no concatenation passing to be done?
					SplitPosn = CycleLastGroup(Pattern,CharLpr);
					if (Pattern[CharLpr+1]=='\0' && SplitPosn==0){
						Adult->Gender=GENDER_MULTIP;
						Adult->MultipMin=1;
						Adult->MultipMax=MAXLEN;
						TempPattern[CharLpr]='\0';
						Adult->Child=NewMatchCollection(TempPattern);
						RegParse(Adult->Child);
					}
					else{
						if(SplitPosn==0) SplitPosn = CharLpr+1;
						CharLpr=RegParseSplit(Adult,SplitPosn,0);
					}
				}

			break;

			case '?':
				
				if(BracketLevel==0){
					//There is no concatenation passing to be done?
					SplitPosn = CycleLastGroup(Pattern,CharLpr);
					if (Pattern[CharLpr+1]=='\0' && SplitPosn==0){
						Adult->Gender=GENDER_MULTIP;
						Adult->MultipMin=0;
						Adult->MultipMax=1;
						TempPattern[CharLpr]='\0';
						Adult->Child=NewMatchCollection(TempPattern);
						RegParse(Adult->Child);
					}
					else{
						if(SplitPosn==0) SplitPosn = CharLpr+1;
						CharLpr=RegParseSplit(Adult,SplitPosn,0);
					}
				}

			break;

			case '}':
			
				if(BracketLevel==0 && IsMultipPattern(TempPattern,CharLpr)){

					SplitPosn=CharLpr;
					while(Pattern[SplitPosn]!='{') SplitPosn--;
					SplitPosn = CycleLastGroup(Pattern,SplitPosn);

					if (Pattern[CharLpr+1]=='\0' && SplitPosn==0){
						
						Adult->Gender=GENDER_MULTIP;
						TempPattern[CharLpr]='\0';

						SplitPosn=CharLpr;
						while(TempPattern[SplitPosn]!=',' && TempPattern[SplitPosn]!='{') SplitPosn--;
						
						Adult->MultipMax=atoi(TempPattern + SplitPosn + 1);

						if(TempPattern[SplitPosn]==','){
							TempPattern[SplitPosn]='\0';
							while(TempPattern[SplitPosn]!='{') SplitPosn--;
							Adult->MultipMin=atoi(TempPattern + SplitPosn + 1);
						}
						else{
							Adult->MultipMin=Adult->MultipMax;
						}

						TempPattern[SplitPosn]='\0';

						Adult->Child=NewMatchCollection(TempPattern);
						RegParse(Adult->Child);
					}
					else{
						if(SplitPosn==0) SplitPosn = CharLpr+1;
						CharLpr=RegParseSplit(Adult,SplitPosn,0);
					}
				}

			break;

			case '*':
				
				if(BracketLevel==0){
					//There is no concatenation passing to be done?
					SplitPosn = CycleLastGroup(Pattern,CharLpr);
					if (Pattern[CharLpr+1]=='\0' && SplitPosn==0){
						Adult->Gender=GENDER_MULTIP;
						Adult->MultipMin=0;
						Adult->MultipMax=MAXLEN;
						TempPattern[CharLpr]='\0';
						Adult->Child=NewMatchCollection(TempPattern);
						RegParse(Adult->Child);
					}
					else{
						if(SplitPosn==0) SplitPosn = CharLpr+1;
						CharLpr=RegParseSplit(Adult,SplitPosn,0);
					}
				}

			break;

			case '(':
				BracketLevel++;
			break; 

			case ')':

				BracketLevel--;
				if(BracketLevel==0 && Pattern[CharLpr+1]=='\0'){
					SplitPosn = CycleLastGroup(TempPattern,CharLpr+1);
					if(SplitPosn==0){
						TempPattern[CharLpr]= '\0';
						TempPattern++;
						Adult->Child=NewMatchCollection(TempPattern);
						TempPattern--;
						Adult->Gender = GENDER_UNBRAC;
						RegParse(Adult->Child);
					}
					else{
						CharLpr=RegParseSplit(Adult,SplitPosn,0);
					}
				}
				else if(BracketLevel==0 && !IsMultip(Pattern[CharLpr+1])){
					CharLpr=RegParseSplit(Adult,CharLpr+1,0);
				}
				
			break;
			
			case '|':
				if(BracketLevel==0){
					CharLpr=RegParseSplit(Adult,CharLpr,1);
					Adult->Gender=GENDER_UNION;
				}
			break;

			default:

			break;
				
		}

		if(Pattern[CharLpr]!='\0') CharLpr++;
	}

}

int RegParseSplit(struct MatchCollection *Adult,int SplitPosn,int SplitOffs){

		int CharLpr;
		
		//Make this node concatenation node.
		Adult->Gender = GENDER_CONCAT;
					
		//Pass first segment of concat pattern as child
		TempPattern[SplitPosn]='\0';
		Adult->Child=NewMatchCollection(TempPattern);

		//Pass second segment of concat pattern as child next
		SplitPosn += SplitOffs;
		CharLpr=SplitPosn;
		while(Adult->Pattern[CharLpr]!='\0'){
			TempPattern[CharLpr-SplitPosn]=Adult->Pattern[CharLpr];
			CharLpr++;
		}
					
		TempPattern[CharLpr-SplitPosn]='\0';
		Adult->Child->Next=NewMatchCollection(TempPattern);

		RegParse(Adult->Child);
		RegParse(Adult->Child->Next);

		return CharLpr;
}

void RegEval(struct MatchCollection *Eval, char *TestString){

	int PatLpr;
	int TestMstrLpr;
	int TestChldLpr;
	int TestExitLpr;
	int TestLpr;

	struct Match *MatchPrntLpr;

	struct Match *MatchChldLpr;
	struct Match *MatchNextLpr;
	struct Match *LastMatch;
	struct Match *TempMatch;

	int Exit;
	int InCharClass;
	int InCharClassExit;
	int HasMatch;
	int Increment;
	int MultipLevel;


	switch(Eval->Gender){

		case GENDER_EVAL:

			TestMstrLpr=0;
			InCharClass=0;
			InCharClassExit=1;
			
			TestExitLpr=0;
			while(!TestExitLpr){
				TestChldLpr=TestMstrLpr;
				Exit=0;
				PatLpr=0;
				
				while(!Exit || InCharClass){
					Exit=1;
					Increment=1;
					switch(Eval->Pattern[PatLpr]){
						case '\0':
							AddMatch(Eval,TestMstrLpr,TestChldLpr,NULL,NULL);
						break;
						case '\\':
							PatLpr++;
							switch(Eval->Pattern[PatLpr]){
								case 'w':
									if(TestString[TestChldLpr]<='Z' && TestString[TestChldLpr]>='A') Exit=0;
									if(TestString[TestChldLpr]<='z' && TestString[TestChldLpr]>='a') Exit=0;
								break;
								case 'd':
									if(TestString[TestChldLpr]<='9' && TestString[TestChldLpr]>='0') Exit=0;
								break;
								case 's':
									if(TestString[TestChldLpr]==' ') Exit=0;
								break;
								case 't':
									if(TestString[TestChldLpr]=='\t') Exit=0;
								break;
								default:
									if(TestString[TestChldLpr]==Eval->Pattern[PatLpr]) Exit=0;
								break;
							}
						break;
						case '[':
							InCharClass=1;
							InCharClassExit=1;
						break;

						case  ']':
							InCharClass=0;
							Exit=InCharClassExit;
						break;

						case '^':
							if(TestChldLpr==0) Exit=0;
							Increment=0;
						break;

						case '$':
							if(TestString[TestChldLpr]=='\0' && Eval->Pattern[PatLpr+1]=='\0') 
								AddMatch(Eval,TestMstrLpr,TestChldLpr,NULL,NULL);
						break;

						case '.':
							if(TestString[TestChldLpr]!='\0') Exit=0;
						break;

						default:
							if(TestString[TestChldLpr]==Eval->Pattern[PatLpr]) Exit=0;
					}
					PatLpr++;
					
					if(!InCharClass) TestChldLpr += Increment;
					InCharClassExit *= Exit;
				}
				TestMstrLpr++;
				if(TestString[TestMstrLpr-1]=='\0') TestExitLpr=1;
			}
			

		break;

		case GENDER_UNION:

			RegEval(Eval->Child,TestString);
			RegEval(Eval->Child->Next,TestString);

			MatchChldLpr=Eval->Child->Matches;
			while(MatchChldLpr!=NULL){
				AddMatch(Eval,MatchChldLpr->FirstChar,MatchChldLpr->NextChar,MatchChldLpr,NULL);
				MatchChldLpr=MatchChldLpr->Next;
			}

			MatchNextLpr=Eval->Child->Next->Matches;
			while(MatchNextLpr!=NULL){
				MatchChldLpr=Eval->Child->Matches;
				HasMatch=0;
				while(MatchChldLpr!=NULL){
					if(MatchChldLpr->FirstChar == MatchNextLpr->FirstChar && MatchChldLpr->NextChar == MatchNextLpr->NextChar)
						HasMatch=1;
					MatchChldLpr=MatchChldLpr->Next;
				}
				if(!HasMatch) AddMatch(Eval,MatchNextLpr->FirstChar,MatchNextLpr->NextChar,MatchNextLpr,NULL);
				MatchNextLpr=MatchNextLpr->Next;
			}

		break;

		case GENDER_CONCAT:
			RegEval(Eval->Child,TestString);
			RegEval(Eval->Child->Next,TestString);

			MatchChldLpr=Eval->Child->Matches;
			while(MatchChldLpr!=NULL){
				MatchNextLpr=Eval->Child->Next->Matches;
				while(MatchNextLpr!=NULL){
					if(MatchChldLpr->NextChar==MatchNextLpr->FirstChar)
						AddMatch(Eval,MatchChldLpr->FirstChar,MatchNextLpr->NextChar,MatchChldLpr,MatchNextLpr);
					MatchNextLpr=MatchNextLpr->Next;
				}
				MatchChldLpr=MatchChldLpr->Next;
			}

		break;

		case GENDER_MULTIP:

			RegEval(Eval->Child,TestString);

			if(Eval->MultipMin <= 0 && 0 <= Eval->MultipMax){
				TestLpr=0;
				AddMatch(Eval,TestLpr,TestLpr,NULL,NULL);
				while(TestString[TestLpr]!='\0'){
					TestLpr++;
					AddMatch(Eval,TestLpr,TestLpr,NULL,NULL);
				}
				MultipLevel=0;
			}
			else{
				MatchChldLpr=Eval->Child->Matches;
				while(MatchChldLpr!=NULL){
					AddMatch(Eval,MatchChldLpr->FirstChar,MatchChldLpr->NextChar,MatchChldLpr,NULL);
					MatchChldLpr=MatchChldLpr->Next;
				}
				MultipLevel=1;
			}

			LastMatch=Eval->Matches;
			while(LastMatch!=NULL){
				MatchPrntLpr=LastMatch;
				LastMatch=NULL;
				while(MatchPrntLpr!=LastMatch){
					MatchChldLpr=Eval->Child->Matches;
					while(MatchChldLpr!=NULL){
						if(MatchChldLpr->NextChar==MatchPrntLpr->FirstChar){
							if(LastMatch==NULL)
								LastMatch = AddMatch(Eval,MatchChldLpr->FirstChar,MatchPrntLpr->NextChar,MatchChldLpr,MatchPrntLpr);
							else
								AddMatch(Eval,MatchChldLpr->FirstChar,MatchPrntLpr->NextChar,MatchChldLpr,MatchPrntLpr);
						}
						MatchChldLpr=MatchChldLpr->Next;
					}
					MatchPrntLpr=MatchPrntLpr->Next;
				}
				MultipLevel++;
				if(MultipLevel==Eval->MultipMin){
					while(Eval->Matches!=LastMatch){
						TempMatch=Eval->Matches;
						Eval->Matches=Eval->Matches->Next;
						DisposeMatch(TempMatch);
					}
				}
				if(MultipLevel==Eval->MultipMax) LastMatch=NULL;
			}

		break;

		case GENDER_UNBRAC:
			
			RegEval(Eval->Child,TestString);
			MatchChldLpr=Eval->Child->Matches;
			while(MatchChldLpr!=NULL){
				TempMatch=AddMatch(Eval,MatchChldLpr->FirstChar,MatchChldLpr->NextChar,MatchChldLpr,NULL);
				AddSubMatch(TempMatch,MatchChldLpr->FirstChar,MatchChldLpr->NextChar);
				MatchChldLpr=MatchChldLpr->Next;
			}

		break;
	}
}


struct MatchCollection *NewMatchCollection(char *Pattern){

	struct MatchCollection *Return;
	
	Return = malloc(sizeof(struct MatchCollection));

	Return->Pattern=malloc(sizeof(char)*MAXLEN);
	memcpy(Return->Pattern,Pattern,MAXLEN);
	Return->Matches=NULL;
	Return->Gender=GENDER_EVAL;
	Return->Child=NULL;
	Return->Next=NULL;

	return Return;
}

void PrintMatches(struct MatchCollection *MatchCol,char *TestString){
	
	struct Match *MatchLpr;
	struct SubMatch *SubMatchLpr;

	int MatchCtr;

	MatchCtr = 0;
	MatchLpr=MatchCol->Matches;

	while(MatchLpr!=NULL){
		printf("Match %d : \t \"",MatchCtr);
		PrintMatch(MatchLpr,TestString);
		printf("\" \n");

		SubMatchLpr=MatchLpr->SubMatches;
		while(SubMatchLpr!=NULL){
			printf("SubMatch: \t \"");
			PrintSubMatch(SubMatchLpr,TestString);
			SubMatchLpr=SubMatchLpr->Next;
			printf("\" \n");
		}


		MatchLpr=MatchLpr->Next;
		MatchCtr++;
	}
}

void PrintMatch(struct Match *Match,char *TestString){

	int CharLpr;

	for(CharLpr=Match->FirstChar;CharLpr<Match->NextChar;CharLpr++){
		printf("%c",TestString[CharLpr]);
	}

}
void PrintSubMatch(struct SubMatch *SubMatch,char *TestString){

	int CharLpr;

	for(CharLpr=SubMatch->FirstChar;CharLpr<SubMatch->NextChar;CharLpr++){
		printf("%c",TestString[CharLpr]);
	}
}

int CycleLastGroup(char *Pattern,int CharLpr){
	
	int BracketVal;
	int SqBracketVal;
	char CurChar;
	int Exit;


	Exit =1;
	BracketVal=0;
	SqBracketVal=0;

	CharLpr--;

	while(CharLpr>=0){

		
		CurChar=Pattern[CharLpr];

		switch(CurChar){
			
			case ')':
				BracketVal++;
			break;

			case '(':
				BracketVal--;
				if (SqBracketVal==0 && BracketVal==0) return CharLpr;
			break;

			case ']':
				SqBracketVal++;
			break;

			case '[':
				SqBracketVal--;
				if (SqBracketVal==0 && BracketVal==0) return CharLpr;
			break;

			case '\\':
				if (Exit==0) return CharLpr;
			
			default:
				if (Exit==0) return CharLpr+1;
				if (SqBracketVal==0 && BracketVal==0) Exit = 0;
		}
		
		CharLpr--;
	}

	return 0;

}

void PrintDebug(struct MatchCollection *MatchColLpr){
	
	struct Match *MatchLpr;

	printf("Pattern is \"%s\"\n",MatchColLpr->Pattern);
	MatchLpr=MatchColLpr->Matches;
	if(MatchLpr!=NULL){
		printf("Matches : ");
		while(MatchLpr!=NULL){
			printf("(%d,%d)",MatchLpr->FirstChar,MatchLpr->NextChar);
			MatchLpr=MatchLpr->Next;
			if(MatchLpr!=NULL) printf(",");
		}
		printf("\n");
	}

	if(MatchColLpr->Child!=NULL){
		printf("Printing child node. \n");
		PrintDebug(MatchColLpr->Child);
	}
	if(MatchColLpr->Next!=NULL) {
		printf("Printing next node. \n");	
		PrintDebug(MatchColLpr->Next);
	}
}


void FreeMatchCollection(struct MatchCollection *MatchColLpr){
	
	struct Match *MatchLpr;
	struct Match *MatchLprTemp;

	if(MatchColLpr->Child!=NULL){
		FreeMatchCollection(MatchColLpr->Child);
	}
	if(MatchColLpr->Next!=NULL) {
		FreeMatchCollection(MatchColLpr->Next);
	}

	MatchLpr=MatchColLpr->Matches;
	while(MatchLpr!=NULL){
		MatchLprTemp=MatchLpr;
		MatchLpr=MatchLpr->Next;
		DisposeMatch(MatchLprTemp);
	}
	free(MatchColLpr->Pattern);
	free(MatchColLpr);

}

struct Match *NewMatch(int StartPos,int EndPos){
	struct Match *Return;
	Return = malloc(sizeof(struct Match));
	Return->FirstChar = StartPos;
	Return->NextChar = EndPos;
	Return->Next = NULL;
	Return->SubMatches = NULL;
	return Return;
}


struct SubMatch *NewSubMatch(int StartPos,int EndPos){
	struct SubMatch *Return;
	Return = malloc(sizeof(struct SubMatch));
	Return->FirstChar = StartPos;
	Return->NextChar = EndPos;
	Return->Next = NULL;
	return Return;
}


void DisposeMatch(struct Match *Match){

	struct SubMatch *SubMatchLpr;
	struct SubMatch *SubMatchLprTemp;

	SubMatchLpr=Match->SubMatches;

	while(SubMatchLpr!=NULL){
		SubMatchLprTemp=SubMatchLpr;
		SubMatchLpr=SubMatchLpr->Next;
		free(SubMatchLprTemp);
	}

	free(Match);
}

struct Match *AddMatch(struct MatchCollection *MatchCol,int StartPos,int EndPos,struct Match *LeftMatch,struct Match *RightMatch){

	struct Match *PrevMatch;
	struct Match *Return;
	struct SubMatch *SubMatchLpr;

	if((PrevMatch = MatchCol->Matches)!=NULL){

		while(PrevMatch->Next!=NULL) PrevMatch=PrevMatch->Next;
		PrevMatch->Next = NewMatch(StartPos,EndPos);
		Return=PrevMatch->Next;
	}
	else{
		MatchCol->Matches = NewMatch(StartPos,EndPos);
		Return=MatchCol->Matches;
	}

	if(LeftMatch!=NULL){
		SubMatchLpr=LeftMatch->SubMatches;
		while(SubMatchLpr!=NULL){
			AddSubMatch(Return,SubMatchLpr->FirstChar,SubMatchLpr->NextChar);
			SubMatchLpr=SubMatchLpr->Next;
		}
	}

	if(RightMatch!=NULL){
		SubMatchLpr=RightMatch->SubMatches;
		while(SubMatchLpr!=NULL){
			AddSubMatch(Return,SubMatchLpr->FirstChar,SubMatchLpr->NextChar);
			SubMatchLpr=SubMatchLpr->Next;
		}
	 }

	return Return;
}

void AddSubMatch(struct Match *Parent,int StartPos,int EndPos){

	struct SubMatch *PrevMatch;
	if((PrevMatch = Parent->SubMatches)!=NULL){
		while(PrevMatch->Next!=NULL) PrevMatch=PrevMatch->Next;
		PrevMatch->Next = NewSubMatch(StartPos,EndPos);
	}
	else{
		Parent->SubMatches = NewSubMatch(StartPos,EndPos);
	}
	 
}


int IsMultip(char Input){
	if (Input == '*' || Input == '+' || Input == '{') return 1;      //Is Decimal Point
	return 0;
}

int IsMultipPattern(char *Pattern,int CharLpr){
	
	int HasNumber;

	HasNumber=0;
	while(CharLpr--){
		if(Pattern[CharLpr]==','){
			if(CharLpr==0) return 0;
			CharLpr--;
			if(Pattern[CharLpr]<'0' || Pattern[CharLpr]>'9') return 0;
			while(CharLpr--){
				if(Pattern[CharLpr]=='{') return 1;
				if(Pattern[CharLpr]<'0' || Pattern[CharLpr]>'9') return 0;
			}
		}
		else if('0'<=Pattern[CharLpr] && Pattern[CharLpr]<='9')
		{
			HasNumber=1;
		}
		else if(Pattern[CharLpr]=='{')
		{
			return HasNumber;
		}
		else
		{
			return 0;
		}
	}

	return 0;
}

char *BlankString(int Size){
	char *Return;
	int ChrLpr;
	Return = malloc(sizeof(char)*MAXLEN);
	for(ChrLpr=0;ChrLpr<MAXLEN;ChrLpr++) Return[ChrLpr] = '\0';
	return Return;
}