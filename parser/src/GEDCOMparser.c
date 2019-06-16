#include "GEDCOMparser.h"
#include "GEDCOMutilities.h"
#include "LinkedListAPI.h"

#define MAX_LINE_LENGTH 255

GEDCOMerror createGEDCOM(char *fileName, GEDCOMobject **obj) {
  int numOfLines = 0;
  char **tempBuffer = NULL;
  char *tempLevel = NULL;
  char *tempTag = NULL;
  char *tempValue = NULL;
  char ch = 'a';
  int index = 0;
  int c = 0;
  int numOfFamIndRecords = 0;

  bool prevReturn = false;

  GEDCOMerror message;

  FILE *savePtr = NULL;
  savePtr = fopen(fileName, "r");

  // Check the file pointer
  if (savePtr == NULL || strstr(fileName, ".ged") == NULL) {
    message.line = -1;
    message.type = INV_FILE;

    return message;
  }

  // Count number of lines and number of characters in each line
  while (!feof(savePtr)) {
    ch = fgetc(savePtr);

    if ((ch == '\n' || ch == '\r') && !prevReturn) {
      numOfLines++;

      if (c > MAX_LINE_LENGTH || c < 3) {
        message.line = numOfLines;
        message.type = INV_RECORD;

        *obj = NULL;

        fclose(savePtr);

        return message;
      }

      prevReturn = true;
      c = 0;

    } else if ((ch == '\n' || ch == '\r') && prevReturn) {
      prevReturn = false;
    } else {
      prevReturn = false;
      c++;
    }
  }
  numOfLines++;

  // Allocate memory for 2D GEDCOM buffer array
  tempBuffer = malloc(sizeof(char *) * numOfLines + 1);

  for (int i = 0; i < numOfLines; i++) {
    tempBuffer[i] = NULL;
    tempBuffer[i] = malloc(sizeof(char) * MAX_LINE_LENGTH + 1);
    tempBuffer[i][0] = ' ';
    strcpy(tempBuffer[i], "");
  }

  rewind(savePtr);

  prevReturn = false;
  c = 0;
  // Store GEDCOM in array
  while (!feof(savePtr)) {
    ch = fgetc(savePtr);

    if ((ch == '\n' || ch == '\r') && !prevReturn) {
      tempBuffer[index][c] = '\0';
      removeTrailingHardReturn(&tempBuffer[index]);
      index++;
      c = 0;
      prevReturn = true;
    } else if ((ch == '\n' || ch == '\r') && prevReturn) {
      prevReturn = false;
    } else {
      tempBuffer[index][c] = ch;
      prevReturn = false;
      c++;
    }
  }

  int originalLines = numOfLines;
  numOfLines--;

  // Validate the file. If there is an error, all memory allocated up to now is
  // free'd and message is returned.
  message = validateFile(tempBuffer, numOfLines);

  if (message.type != OK) {
    for (int i = 0; i < numOfLines; i++) free(tempBuffer[i]);
    free(tempBuffer);

    fclose(savePtr);

    return message;
  }

  if ((*obj = malloc(sizeof(GEDCOMobject))) == NULL) {
    message.type = OTHER_ERROR;
    message.line = -1;
  }

  Link *linker = malloc(sizeof(Link));

  (*obj)->header = malloc(sizeof(Header));
  (*obj)->individuals =
      initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
  (*obj)->families =
      initializeList(&printFamily, &deleteFamily, &compareFamilies);

  for (int i = 0; i < numOfLines; i++) {
    tempLevel = getLevel(tempBuffer[i]);

    if (strcmp(tempLevel, "0") == 0) {
      tempTag = getTag(tempBuffer[i]);
      tempValue = getValue(tempBuffer[i]);

      if (strcmp(tempTag, "HEAD") == 0) {
        loadHead(*obj, tempBuffer);
        free(tempValue);
        free(tempTag);

      } else if (strcmp(tempValue, "SUBM") == 0 && tempTag[0] == '@') {
        loadSubmitter(*obj, tempBuffer, i);
        free(tempValue);
        free(tempTag);

      } else if (strcmp(tempValue, "INDI") == 0 && tempTag[0] == '@') {
        Link newLink;

        newLink.ptr = createIndividualRecord();
        newLink.xrefId = malloc(sizeof(char) * strlen(tempTag) + 1);
        strcpy(newLink.xrefId, tempTag);
        linker[numOfFamIndRecords] = newLink;
        numOfFamIndRecords++;

        loadIndividual(*obj, tempBuffer, i, newLink.ptr);
        linker = realloc(linker, sizeof(Link) * (numOfFamIndRecords + 1));

        insertBack(&(*obj)->individuals, newLink.ptr);

        free(tempTag);
        free(tempValue);

      } else if (strcmp(tempValue, "FAM") == 0 && tempTag[0] == '@') {
        Link newLink;

        newLink.ptr = createFamilyRecord();
        newLink.xrefId = malloc(sizeof(char) * strlen(tempTag) + 1);
        strcpy(newLink.xrefId, tempTag);
        linker[numOfFamIndRecords] = newLink;
        numOfFamIndRecords++;

        loadFamilies(*obj, tempBuffer, i, newLink.ptr);
        linker = realloc(linker, sizeof(Link) * (numOfFamIndRecords + 1));

        insertBack(&(*obj)->families, newLink.ptr);

        free(tempTag);
        free(tempValue);

      } else if (strcmp(tempTag, "TRLR") == 0) {
        free(tempValue);
        free(tempTag);
        free(tempLevel);

        break;

      } else {
        free(tempTag);
        free(tempValue);
      }
    }

    free(tempLevel);
  }

  linkIndividual(*obj, tempBuffer, linker, numOfLines, numOfFamIndRecords);

  // Free temporary memory
  for (int i = 0; i < numOfFamIndRecords; i++) free(linker[i].xrefId);
  free(linker);

  for (int i = 0; i < originalLines; i++) free(tempBuffer[i]);
  free(tempBuffer);

  fclose(savePtr);

  message.line = -1;
  message.type = OK;
  return message;
}

char *printGEDCOM(const GEDCOMobject *obj) {
  char *gedcom = NULL;
  char *next = NULL;

  if (obj == NULL) {
    return "";
  }

  gedcom = malloc(sizeof(char) * 50);
  strcpy(gedcom, "\nPrinting GEDCOM file:\n---------------------\n");

  Family *temp = NULL;

  char version[10];
  sprintf(version, "%.2f", obj->header->gedcVersion);

  gedcom = realloc(
      gedcom, sizeof(char) * (strlen(gedcom) + strlen(obj->header->source) +
                              strlen(version) + 70));
  strcat(gedcom, "\n-----HEADER-----\n");
  strcat(gedcom, "Source: ");
  strcat(gedcom, obj->header->source);
  strcat(gedcom, "\n");
  strcat(gedcom, "Version: ");
  strcat(gedcom, version);
  strcat(gedcom, "\n\n");

  gedcom =
      realloc(gedcom, sizeof(char) * (strlen(gedcom) +
                                      strlen(obj->submitter->submitterName) +
                                      strlen(obj->submitter->address) + 70));
  strcat(gedcom, "\n-----Submitter-----\n");
  strcat(gedcom, "Name: ");
  strcat(gedcom, obj->submitter->submitterName);
  strcat(gedcom, "\n");
  strcat(gedcom, "Address: ");
  strcat(gedcom, obj->submitter->address);
  strcat(gedcom, "\n\n");

  ListIterator families = createIterator(obj->families);
  while ((temp = nextElement(&families)) != NULL) {
    next = printFamily(temp);
    gedcom =
        realloc(gedcom, sizeof(char) * (strlen(gedcom) + strlen(next) + 20));
    strcat(gedcom, next);
    strcat(gedcom, "\n\n");
  }

  return gedcom;
}

void deleteGEDCOM(GEDCOMobject *obj) {
  if (obj == NULL) {
    return;
  }

  clearList(&obj->individuals);
  clearList(&obj->families);
  clearList(&obj->header->otherFields);
  clearList(&obj->submitter->otherFields);

  free(obj->header);
  free(obj->submitter);
  free(obj);
}

char *printError(GEDCOMerror err) {
  char *errorMessage = malloc(sizeof(char) * 70);
  strcpy(errorMessage, "The error code returned is: ");
  char lineNum[3];

  switch (err.type) {
    case OK:
      strcat(errorMessage, "OK");
      break;

    case INV_GEDCOM:
      strcat(errorMessage, "INV_GEDCOM");
      break;

    case INV_FILE:
      strcat(errorMessage, "INV_FILE");
      break;

    case INV_HEADER:
      strcat(errorMessage, "INV_HEADER");
      break;

    case INV_RECORD:
      strcat(errorMessage, "INV_RECORD");
      break;

    case OTHER_ERROR:
      strcat(errorMessage, "OTHER_ERROR");
      break;

    case WRITE_ERROR:
      strcat(errorMessage, "WRITE_ERROR");
      break;
  }

  sprintf(lineNum, "%d", err.line);
  strcat(errorMessage, " on line number: ");
  strcat(errorMessage, lineNum);

  return errorMessage;
}

Individual *findPerson(const GEDCOMobject *familyRecord,
                       bool (*compare)(const void *first, const void *second),
                       const void *person) {
  Individual *toFind = NULL;

  if (familyRecord == NULL || person == NULL) return toFind;

  ListIterator iter = createIterator(familyRecord->individuals);

  while ((toFind = nextElement(&iter)) != NULL) {
    if (compare(person, toFind)) {
      return toFind;
    }
  }

  return NULL;
}

List getDescendants(const GEDCOMobject *familyRecord,
                    const Individual *person) {
  List descendants =
      initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
  Family *temp;

  if (familyRecord == NULL || person == NULL) return descendants;

  if (getLength(person->families) == 0) return descendants;

  ListIterator iter = createIterator(person->families);

  while ((temp = nextElement(&iter)) != NULL) {
    if (person == temp->husband || person == temp->wife) {
      addDescendants(&descendants, temp);
    }
  }

  return descendants;
}

GEDCOMerror writeGEDCOM(char *fileName, const GEDCOMobject *obj) {
  GEDCOMerror message;
  Link *linker = NULL;

  int numOfInd = 0;
  int numOfFams = 0;
  int counter = 0;

  Individual *tempInd = NULL;
  Family *tempFam = NULL;

  FILE *savePtr = NULL;
  savePtr = fopen(fileName, "w");

  writeHead(obj->header, savePtr);
  writeSubmitter(obj->submitter, savePtr);

  numOfInd = getLength(obj->individuals);
  numOfFams = getLength(obj->families);

  int famCounter = 1;
  int indCounter = 1;

  linker = malloc(sizeof(Link) * (numOfInd + numOfFams + 1));

  ListIterator individuals = createIterator(obj->individuals);
  ListIterator families = createIterator(obj->families);

  while ((tempInd = (Individual *)nextElement(&individuals)) != NULL) {
    Link newLink;

    newLink.xrefId = malloc(sizeof(char) * 7);
    sprintf(newLink.xrefId, "@I%d@", indCounter);
    newLink.ptr = tempInd;

    linker[counter] = newLink;

    indCounter++;
    counter++;
  }

  while ((tempFam = (Family *)nextElement(&families)) != NULL) {
    Link newLink;

    newLink.xrefId = malloc(sizeof(char) * 7);
    sprintf(newLink.xrefId, "@F%d@", famCounter);
    newLink.ptr = tempFam;

    linker[counter] = newLink;

    famCounter++;
    counter++;
  }

  writeIndividuals(obj->individuals, savePtr, linker, counter);
  writeFamilies(obj->families, savePtr, linker, counter, famCounter);

  fprintf(savePtr, "0 TRLR\n");

  for (int i = 0; i < counter; i++) free(linker[i].xrefId);
  free(linker);

  message.type = OK;
  message.line = -1;

  fclose(savePtr);

  return message;
}

ErrorCode validateGEDCOM(const GEDCOMobject *obj) {
  ErrorCode error;
  error = OK;

  if (!obj) {
    error = INV_GEDCOM;
    return error;
  }

  Family *tempFamily = NULL;
  Individual *tempIndividual = NULL;
  Field *tempField = NULL;
  Event *tempEvent = NULL;

  const int MAX_VALUE_LENGTH = 200;

  int countIndividuals = 0;
  int countFamilies = 0;

  int numOfIndividuals = 0;
  int numOfFamilies = 0;

  numOfIndividuals = getLength(obj->individuals);
  numOfFamilies = getLength(obj->families);

  if (!obj->header || !obj->submitter) {
    error = INV_GEDCOM;
    return error;
  }

  if (!obj->header->submitter || !obj->header->gedcVersion ||
      !obj->header->encoding) {
    error = INV_HEADER;
    return error;
  }

  if (strlen(obj->submitter->submitterName) < 1) {
    error = INV_RECORD;
    return error;
  }

  if (strlen(obj->header->source) < 1) {
    error = INV_HEADER;
    return error;
  }

  ListIterator individuals = createIterator(obj->individuals);
  while ((tempIndividual = (Individual *)nextElement(&individuals)) != NULL) {
    if (strlen(tempIndividual->givenName) > MAX_VALUE_LENGTH ||
        strlen(tempIndividual->surname) > MAX_VALUE_LENGTH) {
      error = INV_RECORD;
      return error;
    }

    ListIterator eventsIter = createIterator(tempIndividual->events);
    while ((tempEvent = (Event *)nextElement(&eventsIter)) != NULL) {
      if (strlen(tempEvent->date) > MAX_VALUE_LENGTH ||
          strlen(tempEvent->place) > MAX_VALUE_LENGTH) {
        error = INV_RECORD;
        return error;
      }
    }

    ListIterator fieldsIter = createIterator(tempIndividual->otherFields);
    while ((tempField = (Field *)nextElement(&fieldsIter)) != NULL) {
      if (strlen(tempField->value) > MAX_VALUE_LENGTH) {
        error = INV_RECORD;
        return error;
      }
    }

    countIndividuals++;
  }

  ListIterator families = createIterator(obj->families);
  while ((tempFamily = (Family *)nextElement(&families)) != NULL) {
    ListIterator eventsIter = createIterator(tempFamily->events);
    while ((tempEvent = (Event *)nextElement(&eventsIter)) != NULL) {
      if (strlen(tempEvent->date) > MAX_VALUE_LENGTH ||
          strlen(tempEvent->place) > MAX_VALUE_LENGTH) {
        error = INV_RECORD;
        return error;
      }
    }

    ListIterator fieldsIter = createIterator(tempFamily->otherFields);
    while ((tempField = (Field *)nextElement(&fieldsIter)) != NULL) {
      if (strlen(tempField->value) > MAX_VALUE_LENGTH) {
        error = INV_RECORD;
        return error;
      }
    }

    countFamilies++;
  }

  if (countIndividuals != numOfIndividuals || countFamilies != numOfFamilies) {
    error = INV_RECORD;
    return error;
  }

  return error;
}

List getDescendantListN(const GEDCOMobject *familyRecord,
                        const Individual *person, unsigned int maxGen) {
  List descendants =
      initializeList(&printGeneration, &deleteGeneration, &compareGenerations);
  List descendantsNew =
      initializeList(&printGeneration, &deleteGeneration, &compareGenerations);

  Family *temp;
  int countGenerations = 0;

  int maxGenSigned = maxGen;

  const int INIT = 0;

  if (maxGenSigned < 0) return descendants;

  if (maxGen == 0) {
    maxGen = 100;
  }

  List *newGeneration = malloc(sizeof(List) * maxGen);

  if (familyRecord == NULL || person == NULL) return descendants;

  if (getLength(person->families) == 0) return descendants;

  for (int i = 0; i < maxGen; i++) {
    newGeneration[i] =
        initializeList(&printIndividual, &dummyDelete, &sortForDescendants);
    insertBack(&descendants, &newGeneration[i]);
  }

  ListIterator iter = createIterator(person->families);
  while ((temp = nextElement(&iter)) != NULL) {
    if (person == temp->husband || person == temp->wife) {
      addDescendantsN(&descendants, temp, maxGen, INIT);
    }
  }

  List *tempList = NULL;
  ListIterator listIter = createIterator(descendants);
  while ((tempList = (List *)nextElement(&listIter)) != NULL) {
    if (getLength(*tempList) < 1) break;
    countGenerations++;
  }

  for (int i = 0; i < countGenerations; i++) {
    insertBack(&descendantsNew, &newGeneration[i]);
  }

  return descendantsNew;
}

List getAncestorListN(const GEDCOMobject *familyRecord,
                      const Individual *person, int maxGen) {
  List ancestors =
      initializeList(&printGeneration, &deleteGeneration, &compareGenerations);
  List ancestorsNew =
      initializeList(&printGeneration, &deleteGeneration, &compareGenerations);

  Family *temp = NULL;
  Individual *tempChild = NULL;
  int countGenerations = 0;

  const int INIT = 0;

  if (maxGen < 0) return ancestors;

  if (maxGen == 0) maxGen = 100;

  List *newGeneration = malloc(sizeof(List) * maxGen);

  if (familyRecord == NULL || person == NULL) return ancestors;

  if (getLength(person->families) == 0) return ancestors;

  for (int i = 0; i < maxGen; i++) {
    newGeneration[i] =
        initializeList(&printIndividual, &dummyDelete, &sortForDescendants);
    insertBack(&ancestors, &newGeneration[i]);
  }

  ListIterator iter = createIterator(person->families);
  while ((temp = (Family *)nextElement(&iter)) != NULL) {
    ListIterator iterChildren = createIterator(temp->children);
    while ((tempChild = (Individual *)nextElement(&iterChildren)) != NULL) {
      if (tempChild == person) {
        addAncestorsN(&ancestors, temp, maxGen, INIT);
      }
    }
  }

  List *tempList = NULL;
  ListIterator listIter = createIterator(ancestors);
  while ((tempList = (List *)nextElement(&listIter)) != NULL) {
    if (getLength(*tempList) < 1) break;
    countGenerations++;
  }

  for (int i = 0; i < countGenerations; i++) {
    insertBack(&ancestorsNew, &newGeneration[i]);
  }

  return ancestorsNew;
}

char *indToJSON(const Individual *ind) {
  char *JSON = NULL;

  if (ind == NULL) {
    JSON = malloc(sizeof(char) * 2);
    strcpy(JSON, "");
    return JSON;
  }

  JSON = malloc(sizeof(char) *
                (strlen(ind->givenName) + strlen(ind->surname) + 30));

  strcpy(JSON, "{\"givenName\":\"");
  strcat(JSON, ind->givenName);
  strcat(JSON, "\",\"surname\":\"");
  strcat(JSON, ind->surname);
  strcat(JSON, "\"}");

  return JSON;
}

Individual *JSONtoInd(const char *str) {
  if (!str) return NULL;

  Individual *newIndividual = malloc(sizeof(Individual));
  newIndividual->events =
      initializeList(&printEvent, &deleteEvent, &compareEvents);
  newIndividual->families =
      initializeList(&printFamily, &deleteFamily, &compareFamilies);
  newIndividual->otherFields =
      initializeList(&printField, &deleteField, &compareFields);

  if ((newIndividual->givenName = getJSONValue(str, "givenName")) == NULL)
    return NULL;

  if ((newIndividual->surname = getJSONValue(str, "surname")) == NULL)
    return NULL;

  return newIndividual;
}

GEDCOMobject *JSONtoGEDCOM(const char *str) {
  if (str == NULL) return NULL;

  char *tempEncoding = NULL;
  char *tempVersion = NULL;
  char *tempSource = NULL;
  char *tempSubmitterName = NULL;
  char *tempAddress = NULL;

  GEDCOMobject *obj = malloc(sizeof(GEDCOMobject));

  obj->header = malloc(sizeof(Header));
  obj->header->otherFields =
      initializeList(&printField, &deleteField, &compareFields);
  obj->header->submitter = createSubmitter();

  obj->submitter = obj->header->submitter;
  obj->submitter->otherFields =
      initializeList(&printField, &deleteField, &compareFields);

  obj->individuals =
      initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
  obj->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);

  if ((tempSource = getJSONValue(str, "source")) == NULL) return NULL;

  strcpy(obj->header->source, tempSource);

  if ((tempEncoding = getJSONValue(str, "encoding")) == NULL) return NULL;

  if (strcmp(tempEncoding, "ASCII") == 0)
    obj->header->encoding = ASCII;
  else if (strcmp(tempEncoding, "UTF-8") == 0)
    obj->header->encoding = UTF8;
  else if (strcmp(tempEncoding, "ANSEL") == 0)
    obj->header->encoding = ANSEL;
  else
    obj->header->encoding = UNICODE;

  if ((tempVersion = getJSONValue(str, "gedcVersion")) == NULL) return NULL;

  obj->header->gedcVersion = atof(tempVersion);

  if ((tempSubmitterName = getJSONValue(str, "subName")) == NULL) return NULL;
  strcpy(obj->submitter->submitterName, tempSubmitterName);

  if ((tempAddress = getJSONValue(str, "subAddress")) == NULL) return NULL;
  strcpy(obj->submitter->address, tempAddress);

  return obj;
}

void addIndividual(GEDCOMobject *obj, const Individual *toBeAdded) {
  if (!obj || !toBeAdded) return;

  insertBack(&obj->individuals, (void *)toBeAdded);
}

char *iListToJSON(List iList) {
  Individual *tempIndividual = NULL;
  int listSizeMainList = 0;
  int mainListCounter = 0;
  char *JSON = NULL;

  JSON = malloc(sizeof(char) * 3);
  strcpy(JSON, "[");

  listSizeMainList = getLength(iList);

  ListIterator iter = createIterator(iList);
  while ((tempIndividual = (Individual *)nextElement(&iter)) != NULL) {
    JSON = realloc(
        JSON,
        sizeof(char) * (strlen(JSON) + strlen(indToJSON(tempIndividual)) + 6));
    strcat(JSON, indToJSON(tempIndividual));

    mainListCounter++;
    if (mainListCounter != listSizeMainList) strcat(JSON, ",");
  }

  JSON = realloc(JSON, sizeof(char) * (strlen(JSON) + 3));
  strcat(JSON, "]\0");

  return JSON;
}

char *gListToJSON(List gList) {
  List *tempList = NULL;
  Individual *tempIndividual = NULL;
  int listSize = 0;
  int listSizeMainList = 0;
  int mainListCounter = 0;
  int counter = 0;
  char *JSON = NULL;

  JSON = malloc(sizeof(char) * 3);
  strcpy(JSON, "[");

  listSizeMainList = getLength(gList);

  ListIterator iter = createIterator(gList);
  while ((tempList = (List *)nextElement(&iter)) != NULL) {
    listSize = getLength(*tempList);
    strcat(JSON, "[");

    counter = 0;
    ListIterator iterInds = createIterator(*tempList);
    while ((tempIndividual = (Individual *)nextElement(&iterInds)) != NULL) {
      JSON =
          realloc(JSON, sizeof(char) * (strlen(JSON) +
                                        strlen(indToJSON(tempIndividual)) + 6));
      strcat(JSON, indToJSON(tempIndividual));
      counter++;
      if (counter != listSize) strcat(JSON, ",");
    }

    mainListCounter++;
    strcat(JSON, "]");
    if (mainListCounter != listSizeMainList) strcat(JSON, ",");
  }

  JSON = realloc(JSON, sizeof(char) * (strlen(JSON) + 3));
  strcat(JSON, "]\0");

  return JSON;
}

// HELPER FUNCTIONS

void deleteGeneration(void *toBeDeleted) {
  List *a = (List *)toBeDeleted;
  clearList(a);
}

int compareGenerations(const void *first, const void *second) {
  List *a = (List *)first;
  List *b = (List *)second;

  char *aString = toString(*a);
  char *bString = toString(*b);

  return strcmp(aString, bString);
}

char *printGeneration(void *toBePrinted) {
  List *a = (List *)toBePrinted;

  return toString(*a);
}

void deleteEvent(void *toBeDeleted) {
  Event *toDelete = (Event *)toBeDeleted;

  free(toDelete->date);
  toDelete->date = NULL;

  free(toDelete->place);
  toDelete->place = NULL;

  clearList(&toDelete->otherFields);

  free(toDelete);
  toDelete = NULL;
}

int compareEvents(const void *first, const void *second) {
  Event *a = (Event *)first;
  Event *b = (Event *)second;

  return strcmp(a->type, b->type);
}

char *printEvent(void *toBePrinted) {
  Event *toPrint = (Event *)toBePrinted;

  char *toString =
      malloc(sizeof(char) * (strlen(toPrint->date) + strlen(toPrint->place) +
                             strlen(toPrint->type) + 34));
  strcpy(toString, "\nType: ");
  strcat(toString, toPrint->type);
  strcat(toString, " Date: ");
  strcat(toString, toPrint->date);
  strcat(toString, " Place: ");
  strcat(toString, toPrint->place);

  Field *temp;

  toString = realloc(toString, sizeof(char) * (strlen(toString) + 60));
  strcat(toString, "\n-----------\nEvent Other Fields:\n-----------\n");

  ListIterator fields = createIterator(toPrint->otherFields);
  char *next;
  while ((temp = nextElement(&fields)) != NULL) {
    next = printField(temp);
    toString = realloc(toString,
                       sizeof(char) * (strlen(toString) + strlen(next) + 20));
    strcat(toString, next);
    strcat(toString, "\n");
  }

  return toString;
}

void deleteIndividual(void *toBeDeleted) {
  Individual *toDelete = (Individual *)toBeDeleted;

  free(toDelete->givenName);
  free(toDelete->surname);

  clearList(&toDelete->events);
  clearList(&toDelete->otherFields);
  clearList(&toDelete->families);

  free(toDelete);

  return;
}
int compareIndividuals(const void *first, const void *second) {
  Individual *a = (Individual *)first;
  Individual *b = (Individual *)second;

  char *x =
      malloc(sizeof(char) * (strlen(a->givenName) + strlen(a->surname) + 3));
  char *y =
      malloc(sizeof(char) * (strlen(b->givenName) + strlen(b->surname) + 3));

  strcpy(x, a->givenName);
  strcat(x, ",");
  strcat(x, a->surname);

  strcpy(y, b->givenName);
  strcat(y, ",");
  strcat(y, b->surname);

  return strcmp(x, y);
}
char *printIndividual(void *toBePrinted) {
  Individual *person = (Individual *)toBePrinted;
  char *next = NULL;

  char *string = malloc(sizeof(char) * (strlen(person->givenName) +
                                        strlen(person->surname) + 68));

  strcpy(string, "Given Name: ");
  strcat(string, person->givenName);
  strcat(string, " Surname: ");
  strcat(string, person->surname);
  strcat(string, "\n-----------\nFields:\n-----------");

  Field *temp = NULL;
  Event *temp2 = NULL;

  ListIterator fields = createIterator(person->otherFields);
  while ((temp = nextElement(&fields)) != NULL) {
    strcat(string, "\n");
    next = printField(temp);
    string =
        realloc(string, sizeof(char) * (strlen(string) + strlen(next) + 20));
    strcat(string, next);
  }

  string = realloc(string, sizeof(char) * (strlen(string) + 34));
  strcat(string, "\n-----------\nEvents:\n-----------");
  ListIterator events = createIterator(person->events);
  while ((temp2 = nextElement(&events)) != NULL) {
    next = printEvent(temp2);
    string =
        realloc(string, sizeof(char) * (strlen(string) + strlen(next) + 20));
    strcat(string, next);
  }

  return string;
}

void deleteFamily(void *toBeDeleted) {
  Family *toDelete = (Family *)toBeDeleted;

  clearList(&toDelete->events);
  clearList(&toDelete->otherFields);
  clearList(&toDelete->children);

  free(toDelete);
}
int compareFamilies(const void *first, const void *second) {
  Family *a = (Family *)first;
  Family *b = (Family *)second;

  int numX = 0;
  int numY = 0;

  numX = getLength(a->children);
  numY = getLength(b->children);

  if (a->husband != NULL) numX++;

  if (a->wife != NULL) numX++;

  if (b->husband != NULL) numY++;

  if (b->wife != NULL) numY++;

  if (numX > numY)
    return 1;
  else if (numX < numY)
    return -1;

  return 0;
}
char *printFamily(void *toBePrinted) {
  Family *toPrint = (Family *)toBePrinted;

  char *string = malloc(sizeof(char) * 90);

  Field *temp = NULL;
  Event *temp2 = NULL;
  Individual *temp3 = NULL;
  char *next = NULL;

  strcpy(string,
         "\nFamily:\n-----------------------------------------\nHusband:\n~~~~~"
         "~~~\n");

  if (toPrint->husband != NULL) {
    next = printIndividual(toPrint->husband);
    string =
        realloc(string, sizeof(char) * (strlen(string) + strlen(next) + 20));
    strcat(string, next);
  }

  strcat(string, "\n\nWife:\n~~~~~\n");

  if (toPrint->wife != NULL) {
    next = printIndividual(toPrint->wife);
    string =
        realloc(string, sizeof(char) * (strlen(string) + strlen(next) + 20));
    strcat(string, next);
  }

  string = realloc(string, sizeof(char) * (strlen(string) + 30));
  strcat(string, "\n\nChildren:\n~~~~~~~~~");
  ListIterator children = createIterator(toPrint->children);
  while ((temp3 = nextElement(&children)) != NULL) {
    strcat(string, "\n");
    next = printIndividual(temp3);
    string =
        realloc(string, sizeof(char) * (strlen(string) + strlen(next) + 20));
    strcat(string, next);
    strcat(string, "\n-----------\n");
  }

  string = realloc(string, sizeof(char) * (strlen(string) + 30));
  strcat(string, "\n\nOther Fields:\n~~~~~");
  ListIterator fields = createIterator(toPrint->otherFields);
  while ((temp = nextElement(&fields)) != NULL) {
    strcat(string, "\n");
    next = printField(temp);
    string =
        realloc(string, sizeof(char) * (strlen(string) + strlen(next) + 20));
    strcat(string, next);
  }

  string = realloc(string, sizeof(char) * (strlen(string) + 21));
  strcat(string, "\n\nEvents:\n~~~~~");
  ListIterator events = createIterator(toPrint->events);
  while ((temp2 = nextElement(&events)) != NULL) {
    next = printEvent(temp2);
    string =
        realloc(string, sizeof(char) * (strlen(string) + strlen(next) + 20));
    strcat(string, next);
  }

  return string;
}

void deleteField(void *toBeDeleted) {
  Field *toDelete = (Field *)toBeDeleted;

  free(toDelete->tag);
  free(toDelete->value);

  free(toDelete);

  return;
}
int compareFields(const void *first, const void *second) {
  Field *a = (Field *)first;
  Field *b = (Field *)second;

  char *x = malloc(sizeof(char) * (strlen(a->tag) + strlen(a->value) + 3));
  char *y = malloc(sizeof(char) * (strlen(b->tag) + strlen(b->value) + 3));

  strcpy(x, a->tag);
  strcat(x, " ");
  strcat(x, a->value);

  strcpy(y, b->tag);
  strcat(y, " ");
  strcat(y, b->value);

  return strcmp(x, y);
}
char *printField(void *toBePrinted) {
  Field *a = (Field *)toBePrinted;

  char *string =
      malloc(sizeof(char) * (strlen(a->tag) + strlen(a->value)) + 20);
  strcpy(string, "Tag: ");
  strcat(string, a->tag);
  strcat(string, " Value: ");
  strcat(string, a->value);

  return string;
}
