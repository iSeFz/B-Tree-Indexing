#include <bits/stdc++.h>
using namespace std;

/* Global variables used throughout the program */
int fileNodes = 0;
int fileRecords = 0;

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
                "6. Exit\n"
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
            int nodeIndex = insertNewRecordAtIndex((char *)FILENAME.c_str(), recordID, reference);
            if (nodeIndex == -1)
                cout << "\tNo place to insert the record!\n";
            else
                cout << "\tRecord inserted successfully at index #" << nodeIndex << "\n";

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
                cerr << "\tRecord doesn't exist in the index!\n";
            else
                cout << "\tData file reference value ==> " << reference << "\n";
            break;
        }
        case 5: // Displaying the contents of the index file
        {
            cout << setw(30) << "" << "Index File Contents\n";
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
            setNextEmptyIndexAtFile(file);
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
                Record *unmodifiedRecord = record.previousRecord;
                int previousIndex = record.indexInFile;
                while (unmodifiedRecord != NULL)
                {
                    unmodifiedRecord->keys.erase(unmodifiedRecord->keys.rbegin()->first);
                    unmodifiedRecord->keys.insert({recordID, previousIndex});
                    previousIndex = unmodifiedRecord->indexInFile;
                    unmodifiedRecord = unmodifiedRecord->previousRecord;
                }
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

// Deletion implementation class
class IndexDeletion
{
    // Test case
    /*
        3 10
        3 9
        3 8
    */
public:
    // Delete a certain key from the index file by its value
    void deleteKey(char *filename, int &key)
    {
        // Open the file for reading and writing in binary mode
        fstream file(filename, ios::in | ios::out | ios::binary);
        // Save the root node record
        Record root = getRecordAtIndex(file, 1);
        // Delete the key from the record
        deleteKeyFromIndex(file, key, root);
        file.close();
    }

    // Remove certain key from the index & save the updates to the index back to the file
    void deleteKeyFromIndex(fstream &file, int &recordID, Record &record)
    {
        // If the record is an internal node, check for its index pointers
        if (record.isInternalNode)
        {
            // Loop over the keys of the record to find the next record index
            short nextIndex = -1;
            for (auto key : record.keys)
            {
                // If the current key is greater than or equal the record ID
                // Then this is the next record index, save its index and break
                if (key.first >= recordID)
                {
                    nextIndex = key.second;
                    break;
                }
            }
            // If the next index is not found, then the record ID is greater than the last key
            // So, the next index is the last key index in the record
            if (nextIndex == -1)
                nextIndex = record.keys.rbegin()->second;
            // Get the next record by its index
            Record nextRecord = getRecordAtIndex(file, nextIndex, &record);
            // Recursively call the function until reaching the leaf node
            deleteKeyFromIndex(file, recordID, nextRecord);
        }
        // Otherwise, if the record is a leaf node
        else
        {
            // Delete the key from the record
            record.keys.erase(recordID);
            // If the record has less than minimum number of keys, then we must borrow from the left or right node
            if (record.keys.size() < fileNodes / 2 && record.previousRecord != NULL)
            {
                bool borrowedFromLeft = borrowFromLeftNode(file, record, recordID);
                if (!borrowedFromLeft)
                {
                    bool borrowedFromRight = borrowFromRightNode(file, record, recordID);
                    if (!borrowedFromRight)
                    {
                        bool mergedWithLeft = mergeWithLeftNode(file, record, recordID);
                        if (!mergedWithLeft)
                            mergeWithRightNode(file, record, recordID);
                        return;
                    }
                }
            }
            // Otherwise, just update the index pointers of the previous records
            else
                propagateDeletion(file, record, recordID);
            if (record.previousRecord == NULL)
                markRecordAsDeleted(file, record);
            propagateDeletion(file, record, recordID);
            // Save the updates back to the index file
            saveNodesFrom(file, record);
        }
    }

    // Propagate the deletion of a key to the previous records to update their index pointers
    void propagateDeletion(fstream &file, Record &record, int &recordID)
    {
        Record *unmodifiedRecord = record.previousRecord;
        Record *previousRecord = &record;
        while (unmodifiedRecord != NULL)
        {
            unmodifiedRecord->keys.erase(recordID);
            unmodifiedRecord->keys.insert({previousRecord->keys.rbegin()->first, previousRecord->indexInFile});
            previousRecord = unmodifiedRecord;
            unmodifiedRecord = unmodifiedRecord->previousRecord;
        }
    }

    // Borrow a key from the left node if it has more than minimum number of keys
    bool borrowFromLeftNode(fstream &file, Record &record, int &recordID)
    {
        // check borrowing from the left child
        pair<short, short> previousPair = {-1, -1};
        for (auto key : record.previousRecord->keys)
        {
            if (key.first >= recordID)
                break;
            previousPair = key;
        }

        if (previousPair.first == -1)
            return false;

        Record leftChildRecord = getRecordAtIndex(file, previousPair.second, record.previousRecord);
        int largestKeyFromLeftChild = leftChildRecord.keys.rbegin()->first;
        if (leftChildRecord.keys.size() > fileNodes / 2)
        {
            pair<short, short> borrowedKey = *leftChildRecord.keys.rbegin();
            leftChildRecord.keys.erase(borrowedKey.first);
            record.keys.insert(borrowedKey);
            propagateDeletion(file, leftChildRecord, largestKeyFromLeftChild);
            saveNodesFrom(file, record);
            saveNodesFrom(file, leftChildRecord);
            return true;
        }
        return false;
    }

    // Borrow a key from the right node if it has more than minimum number of keys
    bool borrowFromRightNode(fstream &file, Record &record, int &recordID)
    {
        pair<short, short> currentPair = {-1, -1};
        bool found = false;
        for (auto key : record.previousRecord->keys)
        {
            if (found)
            {
                currentPair = key;
                break;
            }
            if (key.first >= recordID)
                found = true;
        }

        if (currentPair.first == -1)
            return false;

        Record rightChildRecord = getRecordAtIndex(file, currentPair.second, record.previousRecord);
        int largestKeyFromRecord = record.keys.rbegin()->first;
        if (rightChildRecord.keys.size() > fileNodes / 2)
        {
            pair<short, short> borrowedKey = *rightChildRecord.keys.begin();
            rightChildRecord.keys.erase(borrowedKey.first);
            record.keys.insert(borrowedKey);
            propagateDeletion(file, record, largestKeyFromRecord);
            saveNodesFrom(file, record);
            saveNodesFrom(file, rightChildRecord);
            return true;
        }
        return false;
    }

    // Merge the record with the left node if it has less than minimum number of keys
    bool mergeWithLeftNode(fstream &file, Record &record, int &recordID)
    {
        pair<short, short> previousPair = {-1, -1};
        for (auto key : record.previousRecord->keys)
        {
            if (key.first >= recordID)
                break;
            previousPair = key;
        }

        if (previousPair.first == -1)
            return false;

        Record leftChildRecord = getRecordAtIndex(file, previousPair.second, record.previousRecord);
        // if no child has enough keys to borrow from, then merge the record with one of its children
        // merge with the left child
        int oldGreatestKeyFromLeftChild = leftChildRecord.keys.rbegin()->first;
        // merge the record with the left child
        for (auto key : record.keys)
            leftChildRecord.keys.insert(key);
        propagateDeletion(file, leftChildRecord, oldGreatestKeyFromLeftChild);
        propagateDeletion(file, record, recordID);
        markRecordAsDeleted(file, record);
        saveNodesFrom(file, leftChildRecord);
        return true;
    }

    // Merge the record with the right node if it has less than minimum number of keys
    bool mergeWithRightNode(fstream &file, Record &record, int &recordID)
    {
        pair<short, short> currentPair = {-1, -1};
        bool found = false;
        for (auto key : record.previousRecord->keys)
        {
            if (found)
            {
                currentPair = key;
                break;
            }
            if (key.first >= recordID)
                found = true;
        }

        if (currentPair.first == -1)
            return false;

        Record rightChildRecord = getRecordAtIndex(file, currentPair.second, record.previousRecord);
        // if no child has enough keys to borrow from, then merge the record with one of its children
        // merge with the right child
        int oldGreatestKeyFromRecord = record.keys.rbegin()->first;
        // merge the record with the right child
        for (auto key : record.keys)
            rightChildRecord.keys.insert(key);
        propagateDeletion(file, rightChildRecord, oldGreatestKeyFromRecord);
        // delete the record
        markRecordAsDeleted(file, record);
        // save the right child
        saveNodesFrom(file, rightChildRecord);
        return true;
    }

    // Update the index file to mark the record as deleted
    void markRecordAsDeleted(fstream &file, Record &record)
    {
        // Clear the keys of the record
        record.keys.clear();
        // Chande the last empty record pointer to point to the index of the deleted record
        short recordSize = 2 * fileNodes + 1;
        short lastEmptyRecordIndex = getNextEmptyIndexFromFile(file);
        if (lastEmptyRecordIndex == -1)
        {
            file.seekp(2, ios::beg);
            file.write((char *)(&record.indexInFile), sizeof(short));
        }
        else
        {
            while (true)
            {
                file.seekp((lastEmptyRecordIndex * recordSize * sizeof(short)) + 2, ios::beg);
                short nextEmptyIndex;
                file.read((char *)(&nextEmptyIndex), sizeof(short));
                if (nextEmptyIndex == -1)
                {
                    file.seekp((lastEmptyRecordIndex * recordSize * sizeof(short)) + 2, ios::beg);
                    file.write((char *)(&record.indexInFile), sizeof(short));
                    break;
                }
                lastEmptyRecordIndex = nextEmptyIndex;
            }
        }

        // Delete the node record from the file
        file.seekp(record.indexInFile * recordSize * sizeof(short), ios::beg);
        short isInternalNode = -1;
        file.write((char *)(&isInternalNode), sizeof(short));
        for (int i = 0; i < fileNodes; i++)
        {
            short cell = -1;
            file.write((char *)(&cell), sizeof(short));
            file.write((char *)(&cell), sizeof(short));
        }
    }

    // Get the next empty node index from the file
    short getNextEmptyIndexFromFile(fstream &file)
    {
        file.seekg(2, ios::beg);
        short nextEmptyIndex;
        file.read((char *)(&nextEmptyIndex), sizeof(short));
        return nextEmptyIndex;
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
    cout << "\tIndex File Created Successfully!\n";
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
    deletion.deleteKey(filename, recordID);
    cout << "\tKey Deleted Successfully!\n";
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
