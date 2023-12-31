#include <bits/stdc++.h>
using namespace std;

/* Global variables used throughout the program */
int fileNodes = 5;
int fileRecords = 10;

/* Mandatory functions prototypes */
void createIndexFile(char *filename, int numberOfRecords, int m);
int insertNewRecordAtIndex(char *filename, int recordID, int reference);
void deleteRecordFromIndex(char *filename, int recordID);
void displayIndexFileContent(char *filename);
int searchARecord(char *filename, int recordID);

// Main function to start the program
int main()
{
    cout << "\tWelcome to the B-Tree Indexing System!";
    string FILENAME = "bTreeIndex.bin";
    while (true)
    {
        cout << "\n1. Create a new index file\n"
                "2. Insert a new record\n"
                "3. Delete a record\n"
                "4. Search for a record\n"
                "5. Display the index file content\n"
                "6- Exit\n"
                "Choose one of the above options >> ";
        int choice;
        cin >> choice;
        switch (choice)
        {
        case 1: // Creating the index file from scratch
        {
            cout << "\tCreateing Index File...\n";
            cout << "Enter the number of records: ";
            cin >> fileRecords;
            cout << "Enter the number of nodes: ";
            cin >> fileNodes;
            createIndexFile((char *)FILENAME.c_str(), fileRecords, fileNodes);
            cout << "\tIndex File Created Successfully!\n";
            break;
        }
        case 2: // Inserting a new record to the index file
        {
            cout << "\tInserting a new record to the index file...\n";
            cout << "Enter the record ID: ";
            int recordID;
            cin >> recordID;
            cout << "Enter the reference value: ";
            int reference;
            cin >> reference;
            // Search for the file to check if the record already exists
            if (searchARecord((char *)FILENAME.c_str(), recordID) != -1)
            {
                cout << "The record already exists in the index!\n";
                break;
            }
            int nodeIndex = insertNewRecordAtIndex((char *)FILENAME.c_str(), recordID, reference);
            if (nodeIndex == -1)
                cout << "There is no place to insert the record!\n";
            else
                cout << "The record was inserted successfully at node " << nodeIndex << "\n";
            break;
        }
        case 3: // Deleting a record from the index file
        {
            cout << "\tDeleting certain record from the index file...\n";
            cout << "Enter the record ID: ";
            int recordID;
            cin >> recordID;
            deleteRecordFromIndex((char *)FILENAME.c_str(), recordID);
            break;
        }
        case 4: // Searching for a record in the index file
        {
            cout << "\tSearching for a record in the index file...\n";
            cout << "Enter the record ID: ";
            int recordID;
            cin >> recordID;
            int reference = searchARecord((char *)FILENAME.c_str(), recordID);
            if (reference == -1)
                cerr << "Record doesn't exist in the index!\n";
            else
                cout << "Data file reference value ==> " << reference << "\n";
            break;
        }
        case 5: // Displaying the contents of the index file
        {
            cout << setw(30) << ""
                 << "Index File Contents\n";
            displayIndexFileContent((char *)FILENAME.c_str());
            break;
        }
        case 6:
        {
            cout << "\tThanks for using our B-Tree Index Application!\n";
            return 0;
        }
        }
    }
}

// Record class to represent a record in the index file
class Record
{
public:
    short indexInFile;
    bool isInternalNode;
    map<short, short> keys;
    Record *previousRecord;
};

// Deletion implementation class
class IndexDeletion
{
public:
    // Delete a certain key from the index file by its value
    void deleteKeyFromIndex(char *filename, int key)
    {
        // Open the file for reading and writing in binary mode
        fstream file(filename, ios::in | ios::out | ios::binary);
        // Save the root node record
        Record root = getRecordAtIndex(file, 1);
        // Delete the key from the record
        deleteKeyFromRecord(file, key, root);
        file.close();
    }

    // Update the previous pointers of the records
    Record deleteKeyFromRecord(fstream &file, int recordID, Record &record)
    {
        // If the record is an internal node, check for previous pointers
        if (record.isInternalNode)
        {
            // Otherwise, update the previous pointers of the records
            short nextIndex = -1;
            for (auto key : record.keys)
            {
                if (key.first >= recordID)
                {
                    nextIndex = key.second;
                    break;
                }
            }
            if (nextIndex == -1)
                nextIndex = record.keys.rbegin()->second;
            Record nextRecord = getRecordAtIndex(file, nextIndex, &record);
            return deleteKeyFromRecord(file, recordID, nextRecord);
        }
        else
        {
            // If the number of keys is greater than the minimum number of keys
            if (record.keys.size() > floor(fileNodes / 2))
            {
                // Delete the key from the record
                record.keys.erase(recordID);
                // Temporary record
                Record tempRecord = record;
                // Update previous records if exists
                while (tempRecord.previousRecord != NULL)
                {
                    // Store the index of the current record
                    short recordIndex = tempRecord.indexInFile;
                    // Erase the last key from the previous record
                    tempRecord.previousRecord->keys.erase(tempRecord.previousRecord->keys.rbegin()->first);
                    // Insert the last key from the current record
                    tempRecord.previousRecord->keys.insert({record.keys.rbegin()->first, recordIndex});
                    // Store the previous record
                    tempRecord = *tempRecord.previousRecord;
                }
                // Save the records back to the file
                saveNodesFrom(file, tempRecord);
            }
            return record;
        }
    }

    // Save the nodes back to the file
    void saveNodesFrom(fstream &file, Record &record)
    {
        int recordSize = 2 * fileNodes + 1;
        file.seekp(record.indexInFile * recordSize * sizeof(short), ios::beg);
        short isInternalNode = record.isInternalNode ? 1 : 0;
        file.write((char *)(&isInternalNode), sizeof(short));
        int i = 0;
        for (auto key : record.keys)
        {
            file.write((char *)(&key.first), sizeof(short));
            file.write((char *)(&key.second), sizeof(short));
            i++;
        }
        for (; i < fileNodes; i++)
        {
            short cell = -1;
            file.write((char *)(&cell), sizeof(short));
            file.write((char *)(&cell), sizeof(short));
        }
        if (record.previousRecord != NULL)
            saveNodesFrom(file, *record.previousRecord);
    }

    // Get the next empty node index from the file
    short getNextEmptyIndexFromFile(fstream &file)
    {
        file.seekg(0, ios::beg);
        file.ignore(sizeof(short));
        short nextEmptyIndex;
        file.read((char *)(&nextEmptyIndex), sizeof(short));
        return nextEmptyIndex;
    }

    // Update the next empty node index in the file
    void setNextEmptyIndexAtFile(fstream &file)
    {
        int recordSize = 2 * fileNodes + 1;
        // get the current empty index (That has been occupied)
        short previousEmptyIndex = getNextEmptyIndexFromFile(file);
        // get the next empty index (That has not been occupied)
        file.seekg(previousEmptyIndex * recordSize * sizeof(short), ios::beg);
        file.ignore(sizeof(short));
        short nextEmptyIndex;
        file.read((char *)(&nextEmptyIndex), sizeof(short));
        // set the next empty index to the file
        file.seekp(0, ios::beg);
        file.ignore(sizeof(short));
        file.write((char *)(&nextEmptyIndex), sizeof(short));
    }

    // Get the record at a certain index from the file
    Record getRecordAtIndex(fstream &file, short recordIndex, Record *previousRecord = NULL)
    {
        int recordSize = 2 * fileNodes + 1;
        file.seekg(recordIndex * recordSize * sizeof(short), ios::beg);
        Record record;
        record.indexInFile = recordIndex;
        short nodeType;
        file.read((char *)(&nodeType), sizeof(short));
        if (nodeType == 1)
            record.isInternalNode = true;
        else
            record.isInternalNode = false;
        pair<short, short> key;
        for (int i = 0; i < fileNodes; i++)
        {
            file.read((char *)(&key.first), sizeof(short));
            file.read((char *)(&key.second), sizeof(short));
            if (key.first == -1 || key.second == -1)
                break;
            record.keys.insert(key);
        }
        record.previousRecord = previousRecord;
        return record;
    }
};

// Insertion implementation class
class IndexInsertion
{
    // Test case
    /*
        2 3 12
        2 7 24
        2 10 48
        2 24 60
        2 14 72
        2 19 84
        2 30 96
        2 15 108
        2 1 120
        2 5 132
        2 2 144
        2 8 156
        2 9 168
        2 6 180
        2 11 192
        2 12 204
        2 17 216
        2 18 228
        2 32 240
    */
public:
    int insert(string &filename, int &recordID, int &reference)
    {
        fstream file(filename, ios::in | ios::out | ios::binary);
        Record root = getRecordAtIndex(file, 1);
        if (root.keys.size() == 0)
        {
            setNextEmptyIndexAtFile(file);
        }
        short index = insertKeyAt(file, recordID, reference, root);
        file.close();
        return index;
    }

    short insertKeyAt(fstream &file, int &recordID, int &reference, Record &record)
    {
        if (record.isInternalNode)
        {
            short nextIndex = -1;
            for (auto key : record.keys)
            {
                if (recordID <= key.first)
                {
                    nextIndex = key.second;
                    break;
                }
            }
            if (nextIndex == -1)
            {
                nextIndex = record.keys.rbegin()->second;
            }
            Record nextRecord = getRecordAtIndex(file, nextIndex, &record);
            return insertKeyAt(file, recordID, reference, nextRecord);
        }
        else
        {
            // if the recordID is greater than the grandest key in the record, then we must modify the its key from the previous record
            if (record.previousRecord != NULL && recordID > record.keys.rbegin()->first)
            {
                record.previousRecord->keys.erase(record.previousRecord->keys.rbegin()->first);
                record.previousRecord->keys.insert({recordID, record.indexInFile});
                saveNodesFrom(file, *record.previousRecord);
            }

            // insert the recordID and reference into the record
            record.keys.insert({recordID, reference});
            if (record.keys.size() > fileNodes)
            {
                return splitRecord(file, record, recordID);
            }
            else
            {
                saveNodesFrom(file, record);
                return record.indexInFile;
            }
        }
    }

    short splitRecord(fstream &file, Record &record, int &recordID)
    {
        Record newRecord;
        newRecord.isInternalNode = record.isInternalNode;
        newRecord.indexInFile = getNextEmptyIndexFromFile(file);
        if (newRecord.indexInFile == -1)
            return -1;
        setNextEmptyIndexAtFile(file);
        int i = 0;
        while (i <= fileNodes / 2)
        {
            newRecord.keys.insert(*record.keys.rbegin());
            record.keys.erase(record.keys.rbegin()->first);
            i++;
        }
        if (record.previousRecord == NULL)
        {
            record.indexInFile = newRecord.indexInFile;
            newRecord.indexInFile = getNextEmptyIndexFromFile(file);
            setNextEmptyIndexAtFile(file);
            Record *newRoot = new Record();
            newRoot->isInternalNode = true;
            newRoot->indexInFile = 1;
            newRoot->keys.insert({record.keys.rbegin()->first, record.indexInFile});
            newRoot->keys.insert({newRecord.keys.rbegin()->first, newRecord.indexInFile});
            record.previousRecord = newRoot;
            newRecord.previousRecord = newRoot;
        }
        else
        {
            Record *previousRecord = record.previousRecord;
            previousRecord->keys[record.keys.rbegin()->first] = record.indexInFile;
            previousRecord->keys[newRecord.keys.rbegin()->first] = newRecord.indexInFile;
            newRecord.previousRecord = previousRecord;
            if (previousRecord->keys.size() > fileNodes)
                splitRecord(file, *previousRecord, recordID);
        }
        saveNodesFrom(file, record);
        saveNodesFrom(file, newRecord);
        // return the index of the record that contains the recordID
        if (record.keys.count(recordID))
            return record.indexInFile;
        else
            return newRecord.indexInFile;
    }

    void saveNodesFrom(fstream &file, Record &record)
    {
        int recordSize = 2 * fileNodes + 1;
        file.seekp(record.indexInFile * recordSize * sizeof(short), ios::beg);
        short isInternalNode = record.isInternalNode ? 1 : 0;
        file.write((char *)(&isInternalNode), sizeof(short));
        int i = 0;
        for (auto key : record.keys)
        {
            file.write((char *)(&key.first), sizeof(short));
            file.write((char *)(&key.second), sizeof(short));
            i++;
        }
        for (; i < fileNodes; i++)
        {
            short cell = -1;
            file.write((char *)(&cell), sizeof(short));
            file.write((char *)(&cell), sizeof(short));
        }
        if (record.previousRecord != NULL)
            saveNodesFrom(file, *record.previousRecord);
    }

    short getNextEmptyIndexFromFile(fstream &file)
    {
        int recordSize = 2 * fileNodes + 1;
        file.seekg(0, ios::beg);
        file.ignore(sizeof(short));
        short nextEmptyIndex;
        file.read((char *)(&nextEmptyIndex), sizeof(short));
        return nextEmptyIndex;
    }

    void setNextEmptyIndexAtFile(fstream &file)
    {
        int recordSize = 2 * fileNodes + 1;
        // get the current empty index (That has been occupied)
        short previousEmptyIndex = getNextEmptyIndexFromFile(file);
        // get the next empty index (That has not been occupied)
        file.seekg(previousEmptyIndex * recordSize * sizeof(short), ios::beg);
        file.ignore(sizeof(short));
        short nextEmptyIndex;
        file.read((char *)(&nextEmptyIndex), sizeof(short));
        // set the next empty index to the file
        file.seekp(0, ios::beg);
        file.ignore(sizeof(short));
        file.write((char *)(&nextEmptyIndex), sizeof(short));
    }

    Record getRecordAtIndex(fstream &file, short recordIndex, Record *previousRecord = NULL)
    {
        int recordSize = 2 * fileNodes + 1;
        file.seekg(recordIndex * recordSize * sizeof(short), ios::beg);
        Record record;
        record.indexInFile = recordIndex;
        short nodeType;
        file.read((char *)(&nodeType), sizeof(short));
        if (nodeType == 1)
            record.isInternalNode = true;
        else
            record.isInternalNode = false;
        pair<short, short> key;
        for (int i = 0; i < fileNodes; i++)
        {
            file.read((char *)(&key.first), sizeof(short));
            file.read((char *)(&key.second), sizeof(short));
            if (key.first == -1 || key.second == -1)
                break;
            record.keys.insert(key);
        }
        record.previousRecord = previousRecord;
        return record;
    }
};

/* Mandatory functions definitions */
// Create the index file with certain number of records
void createIndexFile(char *filename, int numberOfRecords, int m)
{
    // Open the file for output in binary mode
    fstream file(filename, ios::out | ios::binary);
    file.seekp(0, ios::beg);
    // Save the number of nodes and records in the file
    fileNodes = m;
    fileRecords = numberOfRecords;
    int numberOfCells = 2 * fileNodes + 1;
    short initialCell = -1;
    // Loop to write create records with the given number of records
    for (int i = 0; i < fileRecords; i++)
    {
        // Write the initial cell value
        file.write((char *)(&initialCell), sizeof(short));
        // If its the last record, write -1 as the last pointer value
        if (i == fileRecords - 1)
            file.write((char *)(&initialCell), sizeof(short));
        else // Otherwise write the current pointer value
        {
            int nextRecordPointer = i + 1;
            file.write((char *)(&nextRecordPointer), sizeof(short));
        }
        // Write the rest of the cells of the current record
        for (int j = 0; j < numberOfCells - 2; j++)
            file.write((char *)(&initialCell), sizeof(short));
    }
    file.close();
}

// Insert a new record to the index file & return -1 if there is no place to insert the record
// Or return the index of the node where the new record is inserted if the record was inserted successfully
int insertNewRecordAtIndex(char *filename, int recordID, int reference)
{
    IndexInsertion insertion;
    string file(filename);
    return insertion.insert(file, recordID, reference);
}

// Delete a certain key from the index file by its value
void deleteRecordFromIndex(char *filename, int recordID)
{
    // If the record doesn't exist in the index file, then return
    if (searchARecord(filename, recordID) == -1)
    {
        cerr << "\tRecord doesn't exist in the index!\n";
        return;
    }
    // Create an object from the deletion class
    IndexDeletion deletion;
    // Call the delete function
    deletion.deleteKeyFromIndex(filename, recordID);
}

// Display the contents of the index file, each node in a line
void displayIndexFileContent(char *filename)
{
    // Open the file for reading in binary mode
    fstream file(filename, ios::in | ios::binary);
    file.seekg(0, ios::beg);
    int numberOfCells = 2 * fileNodes + 1;
    // Loop to read all records until end of the file
    for (int i = 0; i < fileRecords; i++)
    { // Loop to read the cells of each record
        for (int j = 0; j < numberOfCells; j++)
        { // Output the current cell value
            short currentCell;
            file.read((char *)(&currentCell), sizeof(short));
            cout << setw(5) << currentCell << setw(3) << "|";
        }
        cout << "\n"; // If reached the end of the record, output a new line
    }
    file.close();
}

// Search for a certain record in the index file using its record ID
// Return -1 if the record doesn’t exist in the index
// Or return the reference value to the data file if the record exist on the index
int searchARecord(char *filename, int recordID)
{
    // Open the file for reading in binary mode
    fstream file(filename, ios::in | ios::binary);
    // Size of each record = (2m + 1) cells * 2 bytes per cell
    int recordSize = (2 * fileNodes + 1) * 2;
    // Skip the first record of the index file
    file.seekg(recordSize, ios::beg);
    // Boolean value to track the start of record
    bool startOfRecord = true;
    // Sequential search for the record ID in the index file
    while (true)
    {
        short isLeaf, currentRecordID, nextReference;
        // If it's the start of a new record
        // Read the first cell to see if this is a leaf node or not
        if (startOfRecord)
            file.read((char *)&isLeaf, sizeof(short));
        // Read the record ID of the current record
        file.read((char *)(&currentRecordID), sizeof(short));
        // Read its reference pointer
        file.read((char *)(&nextReference), sizeof(short));
        // Return -1, if the record doesn't exist in the index
        if (currentRecordID == -1)
            return -1;
        // If the current key is greater than or equal the searched key
        if (currentRecordID >= recordID)
        {
            // If the current record is a leaf node
            if (isLeaf == 0)
            {
                // If the current key is equal to the searched key, then this is the data file reference value
                if (currentRecordID == recordID)
                    return nextReference;
                else // Otherwise, the record doesn't exist in the index
                    return -1;
            }
            // Otherwise, this is an index pointer, continue searching
            file.seekg(nextReference * recordSize, ios::beg);
            // Update the startOfRecord to true to start reading from the beginning of the record
            startOfRecord = true;
        }
        // Otherwise, if the current key is smaller than the searched key
        else // Mark the startOfRecord to false to continue searching withing the same record
            startOfRecord = false;
    }
}

void mergeTwoNodes(char *filename, int firstRecord, int secondRecord)
{
    short id, reference;
    ifstream treeFile(filename, ios::in | ios::binary);
    ofstream newFile("newfile.txt", ios::out | ios::binary);
    while (treeFile.tellg() != firstRecord)
    {
        treeFile.read((char *)&id, sizeof(id));
        newFile.write((char *)&id, sizeof(id));
    }
    treeFile.read((char *)&id, sizeof(id));
    newFile.write((char *)&id, sizeof(id));
    while (id != -1)
    {
        treeFile.read((char *)&id, sizeof(id));
        treeFile.read((char *)&reference, sizeof(reference));
        newFile.write((char *)&id, sizeof(id));
        newFile.write((char *)&reference, sizeof(reference));
    }
    int currentPosition = treeFile.tellg();
    treeFile.seekg(secondRecord, ios::beg);
    id = 0;
    // read the bit leaf or not
    treeFile.read((char *)&reference, sizeof(reference));
    while (id != -1)
    {
        treeFile.read((char *)&id, sizeof(id));
        treeFile.read((char *)&reference, sizeof(reference));
        currentPosition += (sizeof(id) + sizeof(reference));
        newFile.write((char *)&id, sizeof(id));
        newFile.write((char *)&reference, sizeof(reference));
    }
    treeFile.seekg(currentPosition, ios::beg);
    while (!treeFile.eof())
    {
        treeFile.read((char *)&id, sizeof(id));
        treeFile.read((char *)&reference, sizeof(reference));
        newFile.write((char *)&id, sizeof(id));
        newFile.write((char *)&reference, sizeof(reference));
    }
    treeFile.close();
    newFile.close();
    remove(filename);
    rename("newfile.txt", filename);
}

/* int main()
{
    fstream test("bTreeIndex.bin", ios::out | ios::binary);
    test.seekp(0, ios::beg);
    short temp[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                    1, 10, 8, 32, 9, -1, -1, -1, -1, -1, -1,
                    0, 1, 120, 2, 144, 3, 12, -1, -1, -1, -1,
                    0, 11, 192, 12, 204, 14, 72, 15, 108, -1, -1,
                    0, 5, 132, 6, 180, 7, 24, -1, -1, -1, -1,
                    0, 8, 156, 9, 168, 10, 48, -1, -1, -1, -1,
                    0, 17, 216, 18, 228, 19, 84, -1, -1, -1, -1,
                    0, 24, 60, 30, 196, 32, 240, -1, -1, -1, -1,
                    1, 3, 2, 7, 4, 10, 5, -1, -1, -1, -1,
                    1, 15, 3, 19, 6, 32, 7, -1, -1, -1, -1};
    for (short el : temp)
        test.write((char *)&el, sizeof(short));
    test.close();
    return 0;
} */
