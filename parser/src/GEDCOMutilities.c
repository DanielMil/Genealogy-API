#include "GEDCOMparser.h"
#include "GEDCOMutilities.h"
#include "LinkedListAPI.h"

char *createGEDCOMJSON(char *string) {
    
    
    int numOfFamilies = 0;
    int numOfIndividuals = 0;
    char *numOfFamiliesChar = NULL;
    char *numOfIndividualsChar = NULL;
    
    numOfFamiliesChar = malloc(sizeof(char) * 6);
    numOfIndividualsChar = malloc(sizeof(char) * 6);
    
    char *toReturn = malloc(sizeof(char) * (1000 + strlen(string)));
    
    GEDCOMobject *obj = NULL;
    GEDCOMerror error = createGEDCOM(string, &obj);
    
    if (error.type != OK) {
        strcpy(toReturn, "");
        return toReturn;
    }
    
    char * gedcVersion = malloc(sizeof(char) * 4);
    sprintf(gedcVersion, "%.2f", obj->header->gedcVersion);
    CharSet tempEncoding = obj->header->encoding;
    
    strcpy(toReturn, "{\"File\" : \"");
    strcat(toReturn, string);
    strcat(toReturn, "\", \"Source\" : \"");
    strcat(toReturn, obj->header->source);
    strcat(toReturn, "\", \"Version\" : \"");
    strcat(toReturn, gedcVersion);
    strcat(toReturn, "\", \"Encoding\" : \"");
    
    if (tempEncoding == ASCII)
        strcat(toReturn, "ASCII");
    else if (tempEncoding == UTF8)
        strcat(toReturn, "UTF-8");
    else if (tempEncoding == ANSEL)
        strcat(toReturn, "ANSEL");
    else
        strcat(toReturn, "UNICODE");
    
    strcat(toReturn, "\", \"SubName\" : \"");
    strcat(toReturn, obj->submitter->submitterName);
    strcat(toReturn, "\", \"SubAdd\" : \"");
    
    if (strlen(obj->submitter->address) < 2) {
        strcat(toReturn, "");
    } else {
        strcat(toReturn, obj->submitter->address);
    }
    
    ListIterator iterFam = createIterator(obj->families);
    Family *tempFam = NULL;
    while ((tempFam = nextElement(&iterFam)) != NULL)
        numOfFamilies++;
    
    ListIterator iterInds = createIterator(obj->individuals);
    Individual *tempInd = NULL;
    while ((tempInd = nextElement(&iterInds)) != NULL)
        numOfIndividuals++;
    
    sprintf(numOfFamiliesChar, "%d", numOfFamilies);
    sprintf(numOfIndividualsChar, "%d", numOfIndividuals);
    
    strcat(toReturn, "\", \"NumInds\" : \"");
    strcat(toReturn, numOfIndividualsChar);
    strcat(toReturn, "\", \"NumFams\" : \"");
    strcat(toReturn, numOfFamiliesChar);
    
    strcat(toReturn, "\"}");
    
    return toReturn;
}

char *createGEDCOMIndividualJSON (char * fileName) {
    
    char* individual = NULL;
    int familySize = 1;
    char *famSize = NULL;
    famSize = malloc(sizeof(char) * 10);
    char *sex = NULL;
    int numOfInds = 0;
    int counter = 0;
    
    GEDCOMobject *obj = NULL;
    GEDCOMerror error = createGEDCOM(fileName, &obj);
    individual = malloc(sizeof(char) * 500);
    
    if (error.type != OK) {
        strcpy(individual, "");
        return individual;
    }
    
    numOfInds = obj->individuals.length;
    
    sex = malloc(sizeof(char) * 10);
    strcpy(sex, "");
    
    strcpy(individual, "[");
    
    ListIterator iterInds = createIterator(obj->individuals);
    Individual *tempInd = NULL;
    Individual *tempInd2 = NULL;
    while ((tempInd = nextElement(&iterInds)) != NULL) {
        
        individual = realloc(individual, sizeof(char) * (strlen(individual) + strlen(tempInd->givenName) + strlen(tempInd->surname) + 100));
        strcat(individual, "{\"givenName\" : \"");
        strcat(individual, tempInd->givenName);
        strcat(individual, "\", \"surname\" : \"");
        strcat(individual, tempInd->surname);
        
        familySize = 1;
        
        ListIterator iterFam = createIterator(tempInd->families);
        Family *tempFam = NULL;
        while ((tempFam = nextElement(&iterFam)) != NULL) {
            if (tempInd == tempFam->husband || tempInd == tempFam->wife) {
                
                if (tempFam->wife && tempFam->husband)
                    familySize++;
                
                ListIterator iterChildren = createIterator(tempFam->children);
                Individual *tempChild = NULL;
                while ((tempChild = nextElement(&iterChildren)) != NULL) {
                    familySize++;
                }
            }
        }
        
        ListIterator iterFields = createIterator(tempInd->otherFields);
        Field *tempField = NULL;
        while ((tempField = nextElement(&iterFields)) != NULL) {
            if (strcmp(tempField->tag, "SEX") == 0 || strcmp(tempField->tag, "sex") == 0 || strcmp(tempField->tag, "Sex") == 0) {
                strcpy(sex, tempField->value);
            }
        }
        
        sprintf(famSize, "%d", familySize);
        strcat(individual, "\", \"familySize\" : \"");
        strcat(individual, famSize);
        strcat(individual, "\", \"sex\" : \"");
        strcat(individual, sex);
        strcat(individual, "\"}");
        
        counter++;
        
        if (counter < numOfInds) {
            strcat(individual, ",");
        }
    }
    
    strcat(individual,"]");
    
    return individual;
}

char * createNewGEDCOM (char * fileName, char *JSON) {
    
    char *result = malloc(sizeof(char) * 50);
    
    GEDCOMobject *obj = NULL;
    
    obj = JSONtoGEDCOM(JSON);
    
    if (!obj) {
        strcpy(result, "Failed to create new GEDCOM. Read error.");
        return result;
    }
    
    GEDCOMerror error = writeGEDCOM(fileName, obj);
    
    if (error.type != OK) {
        strcpy(result, "Failed to create new GEDCOM. Write error.");
        return result;
    }
    
    strcpy(result, "Successfully created new GEDCOM.");
    
    return result;
}

char * addIndividualFrontEnd (char * filename, char *JSON) {

	char *result = malloc(sizeof(char) * 50);
    
    GEDCOMobject *obj = NULL;
    
    createGEDCOM(filename, &obj);
    
    if (!obj) {
        strcpy(result, "Failed to add individual. Read error.");
        return result;
    }

	Individual *newIndividual = JSONtoInd(JSON);

	if (!newIndividual) {
		strcpy(result, "Failed to add individual. Read error.");
        return result;
	}

	addIndividual(obj, newIndividual);

	GEDCOMerror error = writeGEDCOM(filename, obj);
    
    if (error.type != OK) {
        strcpy(result, "Failed to add individual. Write error.");
        return result;
    }

	strcpy(result, "Successfully added individual.");

	return result;
}

char * descendantsInterface (char *filename, char *givenName, char *surname, int maxGen) {

	char *JSON = NULL;

	GEDCOMobject *obj = NULL;
	createGEDCOM(filename, &obj);
	Individual *toGet = NULL; 

	if (!obj) {
		JSON = malloc(sizeof(char) * 2);
		strcpy(JSON, "");
		return JSON; 
	}

	Individual *tempIndividual = NULL;
	ListIterator iter = createIterator(obj->individuals);
	while ((tempIndividual = (Individual*)nextElement(&iter)) != NULL) {
		if (strcmp(tempIndividual->givenName, givenName) == 0 && strcmp(tempIndividual->surname, surname) == 0) {
			toGet = tempIndividual;
			break; 
		}
	}

	if (!toGet) {
		JSON = malloc(sizeof(char) * 50);
		strcpy(JSON, "Individual not found");
		return JSON; 
	}

	List gen = getDescendantListN(obj, toGet, maxGen);

	if (gen.length == 0) {
		JSON = malloc(sizeof(char) * 40);
		strcpy(JSON, "No Descendants");
		return JSON; 
	}

	JSON = gListToJSON(gen);

	if (!JSON) {
		JSON = malloc(sizeof(char) * 50);
		strcpy(JSON, "Error generating results");
		return JSON;
	}


	return JSON;
}

char * ancestorsInterface (char *filename, char *givenName, char *surname, int maxGen) {

	char *JSON = NULL;

	GEDCOMobject *obj = NULL;
	createGEDCOM(filename, &obj);
	Individual *toGet = NULL; 

	if (!obj) {
		JSON = malloc(sizeof(char) * 2);
		strcpy(JSON, "");
		return JSON; 
	}

	Individual *tempIndividual = NULL;
	ListIterator iter = createIterator(obj->individuals);
	while ((tempIndividual = (Individual*)nextElement(&iter)) != NULL) {
		if (strcmp(tempIndividual->givenName, givenName) == 0 && strcmp(tempIndividual->surname, surname) == 0) {
			toGet = tempIndividual;
			break; 
		}
	}

	if (!toGet) {
		JSON = malloc(sizeof(char) * 50);
		strcpy(JSON, "Individual not found");
		return JSON; 
	}

	List gen = getAncestorListN(obj, toGet, maxGen);

	if (gen.length == 0) {
		JSON = malloc(sizeof(char) * 2);
		strcpy(JSON, "No Ancestors");
		return JSON; 
	}

	JSON = gListToJSON(gen);

	if (!JSON) {
		JSON = malloc(sizeof(char) * 2);
		strcpy(JSON, "Error generating results");
		return JSON;
	}


	return JSON;
}

GEDCOMerror validateFile (char **tempBuffer, int numOfLines) {

	GEDCOMerror message;
	message.line = -1;
	message.type = OK;

	int level = 0;
	int nextLevel = 0;
	int tokenCount = 0;

	bool submitterReferenceInHeadRecord = false;
	bool submitterInFile = false;

	bool sourceInHeader = false;
	bool versionInHeader = false;
	bool encodingInHeader = false;

	char *tag, *charLevel, *charNextLevel, *value = NULL;
	char *firstLevel = NULL, *secondLevel = NULL;

	//Check GEDCOM validity
	tag = getTag(tempBuffer[0]);
	if (strcmp(tag, "HEAD") != 0) {
		message.line = -1;
		message.type = INV_GEDCOM;

		return message;
	}
	free(tag);

	tag = getTag(tempBuffer[numOfLines - 1]);
	tag[4] = '\0'; 
	if (strcmp(tag, "TRLR") != 0) {
		message.line = -1;
		message.type = INV_GEDCOM;

		return message;
	}
	free(tag);

	if (numOfLines < 2) {
		message.line = -1;
		message.type = INV_GEDCOM;

		return message;
	}

	firstLevel = getLevel(tempBuffer[0]);

	if (strcmp(firstLevel, "0") == 0) {

		free(firstLevel);

		int i = 1;
		secondLevel = getLevel(tempBuffer[1]);
		level = atoi(secondLevel);
		free(secondLevel);

		while (level != 0) {

			charLevel = getLevel(tempBuffer[i]);
			level = atoi(charLevel);

			charNextLevel = getLevel(tempBuffer[i + 1]);
			nextLevel = atoi(charNextLevel);

			tokenCount = getTokenCount(tempBuffer[i]);
			value = getValue(tempBuffer[i]);
			tag = getTag(tempBuffer[i]);

			if (level < 0 || level > 99) {
				message.line = i + 1;
				message.type = INV_HEADER;

				return message;
			}

			if (nextLevel - level > 1) {
				message.line = i + 2;
				message.type = INV_RECORD;

				return message;
			}

			if (tokenCount < 2) {
				message.line = i + 1;
				message.type = INV_HEADER;

				return message;
			}

			if (strcmp(tag, "SUBM") == 0)
				submitterReferenceInHeadRecord = true;

			if (strcmp(tag, "SOUR") == 0)
				sourceInHeader = true;

			if (strcmp(tag, "VERS") == 0)
				versionInHeader = true;

			if (strcmp(tag, "CHAR") == 0)
				encodingInHeader = true;

			i++;

			free(tag);
			free(value);
			free(charLevel);
			free(charNextLevel);
		}

	} else {
		free(firstLevel);
	}

	if (!submitterReferenceInHeadRecord || !sourceInHeader || !encodingInHeader || !versionInHeader) {
		message.line = -1;
		message.type = INV_HEADER;

		return message;
	}

	for (int i = 0; i < numOfLines - 1; i++) {

		charLevel = getLevel(tempBuffer[i]);
		level = atoi(charLevel);

		if (tempBuffer[i + 1])
			charNextLevel = getLevel(tempBuffer[i + 1]);

		nextLevel = atoi(charNextLevel);
		tokenCount = getTokenCount(tempBuffer[i]);
		value = getValue(tempBuffer[i]);
		tag = getTag(tempBuffer[i]);

		if (level == 0 && (strcmp(tag, "HEAD") != 0) && (strcmp(tag, "TRLR") != 0) && (strcmp(value, "INDI") != 0) && (strcmp(value, "FAM") != 0) && (strcmp(value, "SUBM") != 0)) {
			message.line = i + 1;
			message.type = INV_RECORD;

			return message;
		}

		if (level == 0 && tokenCount >= 4) {
			message.line = i + 1;
			message.type = INV_RECORD;

			return message;
		}

		if (level < 0 || level > 99) {
			message.line = i + 1;
			message.type = INV_RECORD;

			return message;
		}

		if (nextLevel - level > 1) {
			message.line = i + 2;
			message.type = INV_RECORD;

			return message;
		}

		if (tokenCount < 2) {
			message.line = i + 1;
			message.type = INV_RECORD;

			return message;
		}

		if (strcmp(value, "SUBM") == 0 && tag[0] == '@') {
			submitterInFile = true;
		}

		free(tag);
		free(value);
		free(charLevel);
		free(charNextLevel);
	}

	if (!submitterInFile) {
		message.line = -1;
		message.type = INV_GEDCOM;

		return message;
	}

	return message;
}

int getTokenCount(char *string) {

	if (strlen(string) <= 2)
		return 0;

	const char delim[2] = " ";
	char *token;

	char* tempString = malloc(sizeof(char) * strlen(string) + 1);

	int count = 0;

	strcpy(tempString, string);

	token = strtok(tempString, delim);

	while (token != NULL) {
		count++;
		token = strtok(NULL, delim);
	}

	free(tempString);

	return count;
}

char *getTag(char *toParse) {

	if (strlen(toParse) <= 2)
		return NULL;

	char *tag = malloc(sizeof(char) * 6);
	char *tempString = malloc(sizeof(char) * strlen(toParse) + 1);

	const char delim[2] = " ";
	char *token;

	strcpy(tag, "");
	strcpy(tempString, toParse);

	token = strtok(tempString, delim);
	token = strtok(NULL, delim);

	strcat(tag, token);
	strcat(tag, "\0");

	free(tempString);

	return tag;
}

char *getLevel(char *toParse) {

	if (strlen(toParse) <= 2)
		return "";

	char *level = malloc(sizeof(char) * 3);

	char *tempString = malloc(sizeof(char) * strlen(toParse) + 1);
	const char delim[2] = " ";
	char *token;

	strcpy(level, "");
	strcpy(tempString, toParse);

	token = strtok(tempString, delim);

	strcat(level, token);
	strcat(level, "\0");

	free(tempString);

	return level;
}

char *getValue(char *toParse) {

	if (strlen(toParse) <= 2)
		return "";

	char *value = malloc(sizeof(char) * strlen(toParse));
	char *tempString = malloc(sizeof(char) * strlen(toParse) + 1);
	const char delim[2] = " ";
	char *token;

	strcpy(value, "");
	strcpy(tempString, toParse);

	token = strtok(tempString, delim);
	token = strtok(NULL, delim);
	token = strtok(NULL, delim);

	while (token != NULL) {
		strcat(value, token);
		token = strtok(NULL, delim);

		if (token != NULL)
			strcat(value, delim);

	}

	free(tempString);

	return value;
}

int getLevelInt (char *toParse) {

	if (strlen(toParse) <= 2)
		return 0;

	int level = 0;

	if (toParse[0] == '\r') {
		char a = toParse[1];
		level = a - '0';
		return level;
	}

	char *tempString = malloc(sizeof(char) * strlen(toParse) + 1);
	const char delim[2] = " ";
	char *token;

	strcpy(tempString, toParse);

	token = strtok(tempString, delim);

	level = atoi(token);

	free(tempString);

	return level;
}

char *getGivenName (char *toParse) {

	char *tempString = malloc(sizeof(char) * (strlen(toParse) + 1));
	char *givenName = NULL;

	const char delim[3] = " \t";

	strcpy(tempString, toParse);

	char *token = NULL;

	token = strtok(tempString, delim);
	token = strtok(NULL, delim);
	token = strtok(NULL, delim);

	int tokenCount = getTokenCount(toParse);

	if (tokenCount < 3) {
		givenName = malloc(sizeof(char) * 2);
		return "";
	}

	givenName = malloc(sizeof(char) * (strlen(toParse) + 1));
	strcpy(givenName, "");
	strcpy(givenName, token);

	free(tempString);

	return givenName;
}

char *getSurname (char *toParse) {

	char *surname = malloc(sizeof(char) * strlen(toParse) + 1);
	char *tempString = malloc(sizeof(char) * strlen(toParse) + 1);
	const char delim[2] = " ";
	char *token;

	strcpy(surname, "");

	if (getTokenCount(toParse) < 4) {
		return surname;
	}

	strcpy(tempString, toParse);

	token = strtok(tempString, delim);
	token = strtok(NULL, delim);
	token = strtok(NULL, delim);
	token = strtok(NULL, delim);

	while (token != NULL) {
		strcat(surname, token);
		token = strtok(NULL, delim);

		if (token != NULL)
			strcat(surname, delim);
	}

	if (surname[strlen(surname) - 1] == '/')
		surname[strlen(surname) - 1] = '\0';

	if (surname[0] == '/') {
		for (int i = 0; i < strlen(surname) - 1; i++) {
			surname[i] = surname[i + 1];
		}
		surname[strlen(surname) - 1] = '\0';
	}

	free(tempString);

	return surname;
}

void loadHead (GEDCOMobject* obj, char **tempBuffer) {

	int i = 1;
	int tokenCount = 0;

	char *tempTag = NULL;
	char *tempValue = NULL;

	bool loadedSource = false;
	bool loadedVersion = false;
	bool loadedEncoding = false;

	obj->header->otherFields = initializeList(&printField, &deleteField, &compareFields);

	while (1) {

		if (getLevelInt(tempBuffer[i]) == 0)
			break;

		//Count the number of tokens to determine if an entry is a field
		tokenCount = getTokenCount(tempBuffer[i]);
		tempTag = getTag(tempBuffer[i]);

		//Load set tags
		if (strcmp(tempTag, "SOUR") == 0) {

			tempValue = getValue(tempBuffer[i]);
			strcpy(obj->header->source, tempValue);
			loadedSource = true;
			free(tempValue);
			free(tempTag);

		} else if (strcmp(tempTag, "VERS") == 0 && getLevelInt(tempBuffer[i]) == 2) {

			tempValue = getValue(tempBuffer[i]);
			obj->header->gedcVersion = atof(tempValue);
			loadedVersion = true;
			free(tempValue);
			free(tempTag);

		} else if (strcmp(tempTag, "CHAR") == 0) {

			tempValue = getValue(tempBuffer[i]);

			if (strcmp(tempValue, "ASCII") == 0)
				obj->header->encoding = ASCII;
			else if (strcmp(tempValue, "ANSEL") == 0)
				obj->header->encoding = ANSEL;
			else if (strcmp(tempValue, "UNICODE") == 0)
				obj->header->encoding = UNICODE;
			else if (strcmp(tempValue, "UTF-8") == 0)
				obj->header->encoding = UTF8;

			loadedEncoding = true;

			free(tempTag);
			free(tempValue);

		} else if (strcmp(tempTag, "SUBM") == 0) {

			obj->header->submitter = createSubmitter();

			free(tempTag);

		} else {

			if (tokenCount >= 3) {
				createNewField(&obj->header->otherFields, tempBuffer[i]);
			}

			free(tempTag);
		}

		i++;
	}

	if (!loadedEncoding)
		obj->header->encoding = UTF8;
	if (!loadedVersion)
		obj->header->gedcVersion = 1.0;
	if (!loadedSource)
		strcpy(obj->header->source, "");
}

void createNewField (List * l, char *toParse) {

	char *tempTag;
	char *tempValue;

	Field *newField;
	newField = malloc(sizeof(Field));

	tempTag = getTag(toParse);
	newField->tag = tempTag;
	tempValue = getValue(toParse);
	newField->value = tempValue;
	insertFront(l, newField);

}

void loadIndividual(GEDCOMobject* obj, char **tempBuffer, int startIndex, Individual *recordPtr) {

	int i = startIndex + 1;
	int currentLevel, nextLevel = 0;

	bool loadedEventDate = false;
	bool loadedEventPlace = false;

	int j = 0;
	int tokenCount = 0;
	char *tempTag = NULL;
	char *tempValue = NULL;

	recordPtr->givenName = malloc(sizeof(char) * 2);
	recordPtr->surname = malloc(sizeof(char) * 2);
	strcpy(recordPtr->givenName, "");
	strcpy(recordPtr->surname, "");

	while (1) {

		if (getLevelInt(tempBuffer[i]) == 0)
			break;

		//Count the number of tokens to determine if an entry is a field or event
		tokenCount = getTokenCount(tempBuffer[i]);

		tempTag = getTag(tempBuffer[i]);

		//Load set tags
		if (strcmp(tempTag, "NAME") == 0 && tokenCount >= 4) { //Name with first and last name on same line

			free(recordPtr->givenName);
			free(recordPtr->surname);

			tempValue = getValue(tempBuffer[i]);
			recordPtr->surname = getSurname(tempBuffer[i]);
			recordPtr->givenName = getGivenName(tempBuffer[i]);

			if (strstr(tempValue, "Unknown") || strstr(tempValue, "unknown")) {
				strcpy(recordPtr->surname, "Unknown");
				strcpy(recordPtr->surname, "");
			}

			free(tempTag);
			free(tempValue);

		} else if (strcmp(tempTag, "NAME") == 0 && tokenCount == 3) { //Name with only first name

			free(recordPtr->givenName);
			free(recordPtr->surname);

			tempValue = getValue(tempBuffer[i]);
			recordPtr->givenName = getGivenName(tempBuffer[i]);
			recordPtr->surname = malloc(sizeof(char) * 2);
			strcpy(recordPtr->surname, "");

			if (strstr(tempValue, "Unknown") || strstr(tempValue, "unknown")) {
				recordPtr->surname = realloc(recordPtr->surname, sizeof(char) * 10);
				strcpy(recordPtr->surname, "Unknown");
				strcpy(recordPtr->givenName, "");
			}

			free(tempTag);
			free(tempValue);

		} else if (strcmp(tempTag, "NAME") == 0 && tokenCount == 2) { //Name as an event

			j = i;

			currentLevel = getLevelInt(tempBuffer[j]);
			nextLevel = getLevelInt(tempBuffer[j + 1]);

			free(tempTag);

			while (1) {

				tempTag = getTag(tempBuffer[j]);
				tempValue = getValue(tempBuffer[j]);

				if (strcmp(tempTag, "GIVN") == 0) {

					free(recordPtr->givenName);

					recordPtr->givenName = getGivenName(tempBuffer[j]);
					free(tempValue);
					free(tempTag);

				} else if (strcmp(tempTag, "SURN") == 0) {

					free(recordPtr->surname);

					recordPtr->surname = getGivenName(tempBuffer[j]); //In this case, given name is the surname based on how the parser works.
					free(tempValue);
					free(tempTag);

				} else {
					free(tempValue);
					free(tempTag);
				}

				i++; 

				if (nextLevel - currentLevel <= -1) {
					break;
				}

				j++;

				currentLevel = getLevelInt(tempBuffer[j]);
				nextLevel = getLevelInt(tempBuffer[j + 1]);
			}

		} else if (strlen(tempTag) <= 5 && tokenCount == 2) {

			Event *newEvent = malloc(sizeof(Event));
			newEvent->otherFields = initializeList(&printField, &deleteField, &compareFields);
			strcpy(newEvent->type, tempTag);

			j = i + 1;

			loadedEventDate = false;
			loadedEventPlace = false;

			currentLevel = getLevelInt(tempBuffer[j]);
			nextLevel = getLevelInt(tempBuffer[j + 1]);

			free(tempTag);

			while (1) {

				tempTag = getTag(tempBuffer[j]);
				tempValue = getValue(tempBuffer[j]);

				if (strcmp(tempTag, "DATE") == 0) {

					loadedEventDate = true;
					newEvent->date = tempValue;

				} else if (strcmp(tempTag, "PLAC") == 0) {

					loadedEventPlace = true;
					newEvent->place = tempValue;

				} else {

					createNewField(&newEvent->otherFields, tempBuffer[j]);
					free(tempValue);
				}

				if (nextLevel - currentLevel <= -1) {
					free(tempTag);
					break;
				}

				j++;

				free(tempTag);

				currentLevel = getLevelInt(tempBuffer[j]);
				nextLevel = getLevelInt(tempBuffer[j + 1]);
			}

			if (!loadedEventPlace) {
				newEvent->place = malloc(sizeof(char) * 2);
				strcpy(newEvent->place, "");
			}

			if (!loadedEventDate) {
				newEvent->date = malloc(sizeof(char) * 2);
				strcpy(newEvent->date, "");
			}

			insertFront(&recordPtr->events, newEvent);

		} else {

			//Create new field
			if (tokenCount >= 3 && getLevelInt(tempBuffer[i]) == 1 && strcmp(tempTag, "FAMC") != 0 && strcmp(tempTag, "FAMS") != 0)  {
				createNewField(&recordPtr->otherFields, tempBuffer[i]);
			}

			free(tempTag);
		}

		i++;
	}
}

Submitter *createSubmitter() {

	Submitter *newSubmitter = malloc(sizeof(Submitter) + 1000);

	return newSubmitter;
}

Individual *createIndividualRecord() {

	Individual *newIndividual = malloc(sizeof(Individual));
	newIndividual->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
	newIndividual->families = initializeList(&printFamily, &dummyDelete, &compareFamilies);
	newIndividual->otherFields = initializeList(&printField, &deleteField, &compareFields);

	return newIndividual;
}

Family *createFamilyRecord() {

	Family *newFamily = malloc(sizeof(Family));
	newFamily->otherFields = initializeList(&printField, &deleteField, &compareFields);
	newFamily->children = initializeList(&printIndividual, &dummyDelete, &compareIndividuals);
	newFamily->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
	newFamily->wife = NULL;
	newFamily->husband = NULL;

	return newFamily;
}

void loadFamilies(GEDCOMobject* obj, char **tempBuffer, int startIndex, Family *recordPtr) {

	int i = startIndex + 1;
	int tokenCount = 0;
	char *tempLevel = NULL;
	char *tempTag = NULL;
	int j = 0;
	int currentLevel = 0;
	char *tempValue = NULL;
	bool loadedEventDate = false;
	bool loadedEventPlace = false;
	int nextLevel = 0;
	char *nextLevelChar = NULL;
	char *currentLevelChar = NULL;
	char *nextLevelCharInitial = NULL;
	char *currentLevelCharInitial = NULL;


	while (1) {

		tempLevel = getLevel(tempBuffer[i]);
		if (strcmp(tempLevel, "0") == 0) {
			free(tempLevel);
			break;
		}
		free(tempLevel);

		//Count the number of tokens to determine if an entry is a field
		tokenCount = getTokenCount(tempBuffer[i]);
		tempLevel = getLevel(tempBuffer[i]);
		tempTag = getTag(tempBuffer[i]);

		//Create new field
		if (tokenCount >= 3 && strcmp(tempLevel, "1") == 0 && strcmp(tempTag, "HUSB") != 0 && strcmp(tempTag, "WIFE") != 0 && strcmp(tempTag, "CHIL") != 0)  {

			createNewField(&recordPtr->otherFields, tempBuffer[i]);

			free(tempTag);
			free(tempLevel);

		} else if (strlen(tempTag) <= 5 && tokenCount == 2) {

			Event *newEvent = malloc(sizeof(Event));
			newEvent->otherFields = initializeList(&printField, &deleteField, &compareFields);
			strcpy(newEvent->type, tempTag);

			j = i + 1;

			currentLevelCharInitial = getLevel(tempBuffer[j]);
			nextLevelCharInitial = getLevel(tempBuffer[j + 1]);

			nextLevel = atoi(nextLevelCharInitial);
			currentLevel = atoi(currentLevelCharInitial);

			loadedEventDate = false;
			loadedEventPlace = false;

			free(tempTag);

			while (1) {

				tempTag = getTag(tempBuffer[j]);
				tempValue = getValue(tempBuffer[j]);

				if (strcmp(tempTag, "DATE") == 0) {

					loadedEventDate = true;
					newEvent->date = tempValue;

				} else if (strcmp(tempTag, "PLAC") == 0) {

					loadedEventPlace = true;
					newEvent->place = tempValue;

				} else {

					createNewField(&newEvent->otherFields, tempBuffer[j]);
					free(tempValue);
				}

				i++; 
				if (nextLevel - currentLevel <= -1) {
					free(nextLevelChar);
					free(currentLevelChar);
					free(tempTag);
					break;
				}

				j++;

				if (currentLevelChar != NULL)
					free(currentLevelChar);

				if (nextLevelChar != NULL)
					free(nextLevelChar);

				currentLevelChar = getLevel(tempBuffer[j]);
				nextLevelChar = getLevel(tempBuffer[j + 1]);
				currentLevel = atoi(currentLevelChar);
				nextLevel = atoi(nextLevelChar);

				free(tempTag);
			}

			if (!loadedEventPlace) {
				newEvent->place = malloc(sizeof(char) * 2);
				strcpy(newEvent->place, "");
			}

			if (!loadedEventDate) {
				newEvent->date = malloc(sizeof(char) * 2);
				strcpy(newEvent->date, "");
			}


			free(nextLevelCharInitial);
			free(currentLevelCharInitial);

			free(tempLevel);

			insertFront(&recordPtr->events, newEvent);

		} else {
			free(tempTag);
			free(tempLevel);
		}

		i++;
	}
}

void loadSubmitter(GEDCOMobject* obj, char **tempBuffer, int startIndex) {

	int i = startIndex + 1;

	int tokenCount = 0;
	char *tempTag = NULL;
	char *tempValue = NULL;
	char *tempLevel = NULL;

	bool mainSubmitter = true;

	if (!mainSubmitter) {
		return;
	}

	mainSubmitter = false;

	obj->submitter = obj->header->submitter;
	obj->submitter->otherFields = initializeList(&printField, &deleteField, &compareFields);
	strcpy(obj->submitter->address, "");
	strcpy(obj->submitter->submitterName, "");

	while (1) {

		tempLevel = getLevel(tempBuffer[i]);
		if (strcmp(tempLevel, "0") == 0) {
			free(tempLevel);
			break;
		}
		free(tempLevel);

		//Count the number of tokens to determine if an entry is a field
		tokenCount = getTokenCount(tempBuffer[i]);

		//tempLevel = getLevel(tempBuffer[i]);
		tempTag = getTag(tempBuffer[i]);

		//Load set tags
		if (strcmp(tempTag, "NAME") == 0) {

			tempValue = getValue(tempBuffer[i]);
			strcpy(obj->submitter->submitterName, tempValue);
			free(tempValue);

		} else if (strcmp(tempTag, "ADDR") == 0) {

			tempValue = getValue(tempBuffer[i]);
			strcpy(obj->submitter->address, tempValue);
			free(tempValue);

		} else {

			if (tokenCount >= 3) {
				createNewField(&obj->submitter->otherFields, tempBuffer[i]);
			}
		}

		free(tempTag);

		i++;
	}
}

void linkIndividual(GEDCOMobject *obj, char **array, Link *linker, int numOfLines, int numOfFamIndRecords) {

	char *tempTag = NULL;
	char *tempValue = NULL;
	int j = 0;

	char *tempXrefId = NULL;
	int tempLevel = 0;

	for (int i = 0; i < numOfLines; i++) {

		tempTag = getTag(array[i]);
		tempValue = getValue(array[i]);

		if (tempTag[0] == '@' && (strcmp(tempValue, "INDI") == 0 || strcmp(tempValue, "FAM") == 0)) {

			tempXrefId = malloc(sizeof(char) * strlen(tempTag) + 1);
			strcpy(tempXrefId, tempTag);
			j = i + 1;

			free(tempTag);
			free(tempValue);

			while (1) {

				tempTag = getTag(array[j]);
				tempValue = getValue(array[j]);

				//Individual linking to family
				if (strcmp(tempTag, "FAMC") == 0 || strcmp(tempTag, "FAMS") == 0) {

					for (int k = 0; k < numOfFamIndRecords; k++) {

						if (strcmp(linker[k].xrefId, tempXrefId) == 0) {

							for (int a = 0; a < numOfFamIndRecords; a++) {

								if (strcmp(linker[a].xrefId, tempValue) == 0) {
									insertFront(&((Individual*)linker[k].ptr)->families, linker[a].ptr);
								}
							}

						}

					}

					free(tempTag);
					free(tempValue);

				} else if (strcmp(tempTag, "CHIL") == 0) {

					for (int k = 0; k < numOfFamIndRecords; k++) {

						if (strcmp(linker[k].xrefId, tempXrefId) == 0) {

							for (int a = 0; a < numOfFamIndRecords; a++) {

								if (strcmp(linker[a].xrefId, tempValue) == 0) {
									insertFront(&((Family*)linker[k].ptr)->children, linker[a].ptr);
								}
							}

						}

					}

					free(tempTag);
					free(tempValue);

				} else if (strcmp(tempTag, "HUSB") == 0) {

					for (int k = 0; k < numOfFamIndRecords; k++) {

						if (strcmp(linker[k].xrefId, tempXrefId) == 0) {

							for (int a = 0; a < numOfFamIndRecords; a++) {

								if (strcmp(linker[a].xrefId, tempValue) == 0) {
									((Family*)linker[k].ptr)->husband = linker[a].ptr;
								}
							}

						}

					}

					free(tempTag);
					free(tempValue);

				} else if (strcmp(tempTag, "WIFE") == 0) {

					for (int k = 0; k < numOfFamIndRecords; k++) {

						if (strcmp(linker[k].xrefId, tempXrefId) == 0) {

							for (int a = 0; a < numOfFamIndRecords; a++) {

								if (strcmp(linker[a].xrefId, tempValue) == 0) {
									((Family*)linker[k].ptr)->wife = linker[a].ptr;
								}
							}

						}

					}

					free(tempTag);
					free(tempValue);

				} else {
					free(tempValue);
					free(tempTag);
				}

				tempLevel = getLevelInt(array[j]);
				if (tempLevel == 0) {
					break;
				}

				j++;
			}

			free(tempXrefId);

		} else {
			free(tempValue);
			free(tempTag);
		}
	}

}

void addDescendants(List *list, Family *familyToSearch) {

	Individual *toAdd = NULL;
	Individual *tempIndividual = NULL;
	Family *temp = NULL;
	ListIterator iter = createIterator(familyToSearch->children);

	while ((tempIndividual = (Individual*)nextElement(&iter)) != NULL) {

		toAdd = copyIndividual(tempIndividual);

		insertFront(list, toAdd);

		ListIterator iterFamily = createIterator(tempIndividual->families);
		while ((temp = nextElement(&iterFamily)) != NULL) {

			if (tempIndividual == temp->husband || tempIndividual == temp->wife) {
				addDescendants(list, temp);
			}
		}
	}
}

void addDescendantsN(List *list, Family *familyToSearch, int maxIter, int currIter) {

	Individual *toAdd = NULL;
	Individual *tempIndividual = NULL;
	Family *temp = NULL;
	ListIterator iter = createIterator(familyToSearch->children);
	List *tempList = NULL;
	int generation = 0;
	bool alreadyInList = false;

	if (maxIter == 0)
		maxIter = -1;

	if (currIter ==  maxIter)
		return;

	while ((tempIndividual = (Individual*)nextElement(&iter)) != NULL) {

		toAdd = copyIndividual(tempIndividual);

		alreadyInList = checkForDuplicates(list, tempIndividual);

		generation = 0;
		ListIterator iterList = createIterator(*list);
		while ((tempList = (List*)nextElement(&iterList)) != NULL) {
			if (generation == currIter && !alreadyInList) {
				if (strlen(toAdd->surname) < 1) {
					insertBack(tempList, toAdd);
				} else {
					insertSorted(tempList, toAdd);
				}
			}

			generation++;
		}

		ListIterator iterFamily = createIterator(tempIndividual->families);
		while ((temp = nextElement(&iterFamily)) != NULL) {

			if (tempIndividual == temp->husband || tempIndividual == temp->wife) {
				addDescendantsN(list, temp, maxIter, currIter + 1);
			}
		}

		alreadyInList = false;
	}
}

bool checkForDuplicates (List *container, Individual *toVerify) {

	List *tempList = NULL;
	Individual *tempIndividual = NULL;

	ListIterator iter = createIterator(*container);
	while ((tempList = (List*)nextElement(&iter)) != NULL) {

		ListIterator iterInds = createIterator(*tempList);
		while ((tempIndividual = (Individual*)nextElement(&iterInds)) != NULL) {

			if (toVerify == tempIndividual) {
				return true;
			}
		}

	}

	return false;
}

void addAncestorsN(List *list, Family *familyToSearch, int maxIter, int currIter) {

	//Individual *toAdd = NULL;
	Family *temp = NULL;
	Individual *tempChild = NULL;
	List *tempList = NULL;
	int generation = 0;
	bool alreadyInListHusband = false;
	bool alreadyInListWife = false;

	if (maxIter == 0)
		maxIter = -1;

	if (currIter ==  maxIter)
		return;

	alreadyInListHusband = checkForDuplicates(list, familyToSearch->husband);
	alreadyInListWife = checkForDuplicates(list, familyToSearch->wife);

	if (familyToSearch->husband) {
		//toAdd = copyIndividual(familyToSearch->husband);

		generation = 0;
		ListIterator iterList = createIterator(*list);
		while ((tempList = (List*)nextElement(&iterList)) != NULL) {
			if (generation == currIter && !alreadyInListHusband) {

				if (strlen(familyToSearch->husband->surname) < 1) {
					insertBack(tempList, familyToSearch->husband);
				} else {
					insertSorted(tempList, familyToSearch->husband);
				}
			}

			generation++;
		}

		ListIterator iter = createIterator(familyToSearch->husband->families);
		while ((temp = (Family*)nextElement(&iter)) != NULL) {

			ListIterator iterChildren = createIterator(temp->children);
			while ((tempChild = (Individual*)nextElement(&iterChildren)) != NULL) {

				if (tempChild == familyToSearch->husband) {
					addAncestorsN(list, temp, maxIter, currIter + 1);
				}
			}
		}
	}

	if (familyToSearch->wife) {
		//toAdd = copyIndividual(familyToSearch->wife);

		generation = 0;
		ListIterator iterList = createIterator(*list);
		while ((tempList = (List*)nextElement(&iterList)) != NULL) {
			if (generation == currIter && !alreadyInListWife) {

				if (strlen(familyToSearch->wife->surname) < 1) {
					insertBack(tempList, familyToSearch->wife);
				} else {
					insertSorted(tempList, familyToSearch->wife);
				}
			}

			generation++;
		}

		ListIterator iter = createIterator(familyToSearch->wife->families);
		while ((temp = (Family*)nextElement(&iter)) != NULL) {

			ListIterator iterChildren = createIterator(temp->children);
			while ((tempChild = (Individual*)nextElement(&iterChildren)) != NULL) {

				if (tempChild == familyToSearch->wife) {
					addAncestorsN(list, temp, maxIter, currIter + 1);
				}
			}
		}
	}

}


int sortForDescendants(const void *a, const void *b) {

	Individual *first = (Individual*)a;
	Individual *second = (Individual*)b;

	if (strcmp(first->surname, second->surname) > 0)
		return 1;
	else if (strcmp(first->surname, second->surname) < 0)
		return -1;
	else
		return strcmp(first->givenName, second->givenName);

}

/*Function makes a deep copy of an individual object
 *@param Individual object to copy (source)
 *@return New Individual objet created (destination)
 */
Individual *copyIndividual (Individual *tempIndividual) {

	Individual *toAdd = NULL;
	Family *temp = NULL;
	Field *tempField = NULL;
	Event *tempEvent = NULL;

	toAdd = malloc(sizeof(Individual));
	toAdd->givenName = malloc(sizeof(char) * strlen(tempIndividual->givenName) + 1);
	toAdd->surname = malloc(sizeof(char) * strlen(tempIndividual->surname) + 1);

	strcpy(toAdd->givenName, tempIndividual->givenName);
	strcpy(toAdd->surname, tempIndividual->surname);

	toAdd->otherFields = initializeList(&printField, &deleteField, &compareFields);
	toAdd->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
	toAdd->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);

	ListIterator fields = createIterator(tempIndividual->otherFields);
	ListIterator indEvents = createIterator(tempIndividual->events);
	ListIterator families = createIterator(tempIndividual->families);

	while ((temp = nextElement(&families)) != NULL) {

		insertFront(&toAdd->families, temp);
	}


	while ((tempField = nextElement(&fields)) != NULL) {

		Field *newField;
		newField = malloc(sizeof(Field));

		newField->tag = malloc(sizeof(char) * strlen(tempField->tag) + 1);
		newField->value = malloc(sizeof(char) * strlen(tempField->value) + 1);

		strcpy(newField->tag, tempField->tag);
		strcpy(newField->value, tempField->value);

		insertFront(&toAdd->otherFields, newField);

	}

	while ((tempEvent = nextElement(&indEvents)) != NULL) {

		Event *newEvent;
		newEvent = malloc(sizeof(Event));

		if (tempEvent->date != NULL) {
			newEvent->date = malloc(sizeof(char) * strlen(tempEvent->date) + 1);
			strcpy(newEvent->date, tempEvent->date);
		} else {
			newEvent->date = malloc(sizeof(char) * 2);
			strcpy(tempEvent->date, "");
		}

		if (tempEvent->place != NULL) {
			newEvent->place = malloc(sizeof(char) * strlen(tempEvent->place) + 1);
			strcpy(newEvent->place, tempEvent->place);
		} else {
			newEvent->place = malloc(sizeof(char) * 2);
			strcpy(tempEvent->place, "");
		}

		strcpy(newEvent->type, tempEvent->type);

		newEvent->otherFields = initializeList(&printField, &deleteField, &compareFields);
		ListIterator eventFields = createIterator(tempEvent->otherFields);

		while ((tempField = nextElement(&eventFields)) != NULL) {

			Field *newField;
			newField = malloc(sizeof(Field));

			newField->tag = malloc(sizeof(char) * strlen(tempField->tag) + 1);
			newField->value = malloc(sizeof(char) * strlen(tempField->value) + 1);

			strcpy(newField->tag, tempField->tag);
			strcpy(newField->value, tempField->value);

			insertFront(&toAdd->otherFields, newField);
		}

		insertFront(&toAdd->events, newEvent);
	}

	return toAdd;
}

void removeTrailingHardReturn(char **string) {

	char *modify = (*string);

	int i = strlen(modify) - 1;

	if (i <= 0)
		return;

	while (modify[i] == '\n' || modify[i] == '\r') {
		modify[i] = '\0';
		i--;
	}
}


void dummyDelete(void* toBeDeleted) {
}

void writeHead (Header *toPrint, FILE *savePtr) {

	fprintf(savePtr, "0 HEAD\n");
	fprintf(savePtr, "1 SOUR %s\n", toPrint->source);
	fprintf(savePtr, "1 GEDC\n");
	fprintf(savePtr, "2 VERS %.2lf\n", toPrint->gedcVersion);
	fprintf(savePtr, "2 FORM LINEAGE-LINKED\n");

	switch (toPrint->encoding) {

	case ANSEL:
		fprintf(savePtr, "1 CHAR ANSEL\n");
		break;

	case UTF8:
		fprintf(savePtr, "1 CHAR UTF-8\n");
		break;

	case UNICODE:
		fprintf(savePtr, "1 CHAR UNICODE\n");
		break;

	case ASCII:
		fprintf(savePtr, "1 CHAR ASCII\n");
		break;
	}

	fprintf(savePtr, "1 SUBM @U1@\n");
}

void writeSubmitter (Submitter *toPrint, FILE *savePtr) {

	fprintf(savePtr, "0 @U1@ SUBM\n");
	fprintf(savePtr, "1 NAME %s\n", toPrint->submitterName);

	if (strlen(toPrint->address) > 0) {
		fprintf(savePtr, "1 ADDR %s\n", toPrint->address);
	}
}

void writeIndividuals (List individuals, FILE *savePtr, Link *linker, int numOfLinks) {

	Individual *tempInd = NULL;
	Event *tempEvent = NULL;
	Field *tempField = NULL;
	Family *tempFam = NULL;

	ListIterator individualsIter = createIterator(individuals);

	int counter = 0;

	while ((tempInd = (Individual*)nextElement(&individualsIter)) != NULL) {

		fprintf(savePtr, "0 %s INDI\n", linker[counter].xrefId);
		fprintf(savePtr, "1 NAME %s /%s/\n", tempInd->givenName, tempInd->surname);

		ListIterator fieldsIter = createIterator(tempInd->otherFields);
		while ((tempField = (Field*)nextElement(&fieldsIter)) != NULL) {
			fprintf(savePtr, "1 %s %s\n", tempField->tag, tempField->value);
		}

		ListIterator eventsIter = createIterator(tempInd->events);
		while ((tempEvent = (Event*)nextElement(&eventsIter)) != NULL) {
			fprintf(savePtr, "1 %s\n", tempEvent->type);
			fprintf(savePtr, "2 DATE %s\n", tempEvent->date);
			fprintf(savePtr, "2 PLAC %s\n", tempEvent->place);
		}

		ListIterator famsIter = createIterator(tempInd->families);
		while ((tempFam = (Family*)nextElement(&famsIter)) != NULL) {

			if (tempFam->husband == tempInd || tempFam->wife == tempInd) {

				for (int i = 0; i < numOfLinks; i++) {

					if (linker[i].ptr == tempFam) {
						fprintf(savePtr, "1 FAMS %s\n", linker[i].xrefId);
					}
				}

			} else {

				for (int i = 0; i < numOfLinks; i++) {

					if (linker[i].ptr == tempFam) {
						fprintf(savePtr, "1 FAMC %s\n", linker[i].xrefId);
					}
				}
			}
		}

		counter++;
	}
}

void writeFamilies (List families, FILE *savePtr, Link *linker, int numOfLinks, int numOfFams) {

	Family *tempFam = NULL;
	Event *tempEvent = NULL;
	Individual *tempIndividual = NULL;
	int counter = 0;

	counter = numOfLinks - numOfFams + 1;

	ListIterator famsIter = createIterator(families);
	while ((tempFam = (Family*)nextElement(&famsIter)) != NULL) {

		fprintf(savePtr, "0 %s FAM\n", linker[counter].xrefId);

		ListIterator eventsIter = createIterator(tempFam->events);
		while ((tempEvent = (Event*)nextElement(&eventsIter)) != NULL) {
			fprintf(savePtr, "1 %s\n", tempEvent->type);
			fprintf(savePtr, "2 DATE %s\n", tempEvent->date);
			fprintf(savePtr, "2 PLAC %s\n", tempEvent->place);
		}

		for (int i = 0; i < numOfLinks; i++) {

			if (tempFam->husband == linker[i].ptr) {

				fprintf(savePtr, "1 HUSB %s\n", linker[i].xrefId);

			} else if (tempFam->wife == linker[i].ptr) {

				fprintf(savePtr, "1 WIFE %s\n", linker[i].xrefId);
			}
		}

		ListIterator childrenIter = createIterator(tempFam->children);
		while ((tempIndividual = (Individual*)nextElement(&childrenIter)) != NULL) {

			for (int i = 0; i < numOfLinks; i++) {

				if (tempIndividual == linker[i].ptr) {

					fprintf(savePtr, "1 CHIL %s\n", linker[i].xrefId);
				}
			}
		}

		counter++;
	}
}

char *getJSONValue (const char *JSONObject, const char *key) {

	char *value = NULL;
	char *loc = NULL;
	int countLength = 0;
	int j = 0;

	if ((loc = strstr(JSONObject, key)) == NULL) {

		return NULL;

	} else {

		for (int i = strlen(key) + 3; i < strlen(loc); i++) {
			if (loc[i] == '\"')
				break;
			countLength++;
		}

	}

	value = malloc(sizeof(char) * (countLength + 1));

	for (int i = strlen(key) + 3; i < strlen(loc); i++) {
		value[j] = loc[i];
		j++;

		if (loc[i] == '\"')
			break;
	}

	value[countLength] = '\0';

	return value;
}
