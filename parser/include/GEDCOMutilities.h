#ifndef GEDCOMUTILITIES_H
#define GEDCOMUTILITIES_H 

#define MAX_LINE_LENGTH 255

typedef struct {
	void *ptr;
	char *xrefId;
} Link;  

void removeTrailingHardReturn(char **string); 
void loadHead (GEDCOMobject* obj, char **tempBuffer);
GEDCOMerror validateFile (char **tempBuffer, int numOfLines); 
char *getValue(char *toParse); 
char *getTag(char *toParse);
char *getLevel(char *toParse);
Submitter *createSubmitter(); 
void loadSubmitter(GEDCOMobject* obj, char **tempBuffer, int startIndex);
void createNewField (List * l, char *toParse); 
int getTokenCount(char *string);
void loadIndividual(GEDCOMobject* obj, char **tempBuffer, int startIndex, Individual *recordPtr);
Individual *createIndividualRecord();    
char *getGivenName (char *toParse); 
char *getSurname (char *toParse);
Family *createFamilyRecord(); 
void loadFamilies(GEDCOMobject* obj, char **tempBuffer, int startIndex, Family *recordPtr);
void linkIndividual(GEDCOMobject *obj, char **array, Link *linker, int numOfLines, int numOfFamIndRecords); 
void addDescendants(List *list, Family *familyToSearch); 
void dummyDelete(void* toBeDeleted); 
int getLevelInt (char *toParse); 
void writeHead (Header *toPrint, FILE *savePtr); 
void writeSubmitter (Submitter *toPrint, FILE *savePtr);
void writeIndividuals (List Individuals, FILE *savePtr, Link *linker, int numOfLinks);  
void writeFamilies (List families, FILE *savePtr, Link *linker, int numOfLinks, int numOfFams); 
Individual *copyIndividual (Individual *tempIndividual); 
void addDescendantsN(List *list, Family *familyToSearch, int maxIter, int currIter); 
int sortForDescendants(const void *a, const void *b); 
void addAncestorsN(List *list, Family *familyToSearch, int maxIter, int currIter); 
bool checkForDuplicates (List *container, Individual *toVerify); 
char *getJSONValue (const char *JSONObject, const char *key);  
char *createGEDCOMJSON(char *string); 
char *createGEDCOMIndividualJSON (char * fileName);
char * createNewGEDCOM (char * fileName, char *JSON);
char * addIndividualFrontEnd (char * fileName, char *JSON);
char * ancestorsInterface (char *filename, char *givenName, char *surname, int maxGen);
char * descendantsInterface (char *filename, char *givenName, char *surname, int maxGen); 

#endif
