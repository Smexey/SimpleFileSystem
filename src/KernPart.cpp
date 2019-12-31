#include "KernPart.h"
#include "KernFile.h"
#define _CRT_SECURE_NO_WARNINGS
#include "file.h"
#include <iostream>
using namespace std;


FileControl::FileControl(char mode,char * n,Entry* ent,ClusterNo pos, BytesCnt off):
	fname(n),dirPos(pos),dirOff(off),readers(0),writers(0),readSem(0),writeSem(0),entry(*ent),waitWriters(0),waitReaders(0)
{

    readSem=CreateSemaphore(0,0,100,0);
    writeSem=CreateSemaphore(0,0,1,0);
	
	//kopira iz bafera
    
    if(mode=='r')
    {
        readers++;
    }
    if(mode=='w' || mode=='a')
    {
        writers++;
    }
}



void KernPart::close(KernFile* kernfile) {
	CriticalSection lck(myPartSem);

	if (kernfile->getFileSize() != kernfile->fc->getEntry()->size) {
		//cout << "WROTE FILE TO DISK AFTER CLOSING: " << kernfile;
		kernfile->fc->getEntry()->size = kernfile->getFileSize();
		readCluster(kernfile->fc->dirPos, kernfile->helpCache);
		((Entry*)(kernfile->helpCache))[kernfile->fc->dirOff].size = kernfile->getFileSize();;
		writeCluster(kernfile->fc->dirPos, (kernfile->helpCache));
	}

	string name = kernfile->fc->fname;
	//cout << "=====================closing: " << name;

	map<string, FileControl*>::iterator it = mapOpen.find(name);
	if (it != mapOpen.end()) {
		if (it->second->readers < 2 && it->second->writers == 0 && it->second->waitWriters == 0) {
			delete (it->second);
			mapOpen.erase(it);

			if (mapOpen.empty() && marked)
				signal(deleteSem);
			return;
		}
		else if (it->second->writers < 2 && it->second->readers == 0 && it->second->waitWriters == 0 && it->second->waitReaders == 0) {
			delete (it->second);
			mapOpen.erase(it);

			if (mapOpen.empty() && marked)
				signal(deleteSem);
			return;

		}
	}

	if (it->second->readers > 0)
	{
		it->second->readers--;
		if (it->second->readers == 0)
			if (it->second->waitWriters > 0)
			{
				it->second->waitWriters--;
				signal(it->second->writeSem);
			}
		return;
	}
	else {
		it->second->writers--;
		if (it->second->waitWriters > 0)
		{
			it->second->waitWriters--;
			signal(it->second->writeSem);
		}
		else {
			while (it->second->waitReaders > 0)
			{
				it->second->waitReaders--;
				signal(it->second->readSem);
			}
		}

	}
}

KernPart::KernPart(Partition * part):part(part)
{
    myPartSem = CreateSemaphore(0, 1, 1, 0);
	deleteSem = CreateSemaphore(0, 0, 1, 0);

    bitVect=new BitVector(part->getNumOfClusters(),this);
    part->readCluster(bitVect->getRootPosition(), rootCache);
    
}

char KernPart::format()
{
	CriticalSection lck(myPartSem);

    bitVect->format();
    for(int i=0; i<ClusterSize;i++)
        rootCache[i]=0;
    marked=false;
    return 1;
}

bool KernPart::mark()
{
	CriticalSection lck(myPartSem);

    if(marked) return false;
    marked=true;

    if(mapOpen.empty())
        return true;

    signalAndWait(myPartSem,deleteSem);
	wait(myPartSem);
    return true;
}

int KernPart::readCluster(ClusterNo n, char* buffer) { return part->readCluster(n, buffer); }

ClusterNo KernPart::getNumOfClusters() const { return part->getNumOfClusters(); }

int KernPart::writeCluster(ClusterNo n, const char* buffer) { return part->writeCluster(n, buffer); }

FileControl* KernPart::makeNewFile(char* fname,char mode)
{
    ClusterNo* rootB=(ClusterNo*)rootCache;
    Entry* clEnt;
    ClusterNo* clH;
    char rootBuf1[ClusterSize];
    char rootBuf2[ClusterSize];
	for (ClusterNo i = 0; i < ClusterRootNo; i++) {
		if (rootB[i] != 0)
		{
			readCluster(rootB[i], rootBuf1);
			if (i < ClusterRootNo / 2)
			{
				clEnt = (Entry*)rootBuf1;
				for (ClusterNo j = 0; j < ClusterEntryNo; j++)
					if (freeEntry(clEnt + j))
					{
						//cout << endl << "made new file on entry: " << j << endl;
						writeEntryToBuffer(fname, rootBuf1, rootB[i], j);
						return new FileControl(mode, fname, clEnt + j, rootB[i], j);
					}
			}
			else
			{
				clH = (ClusterNo*)rootBuf1;
				for (ClusterNo j = 0; j < ClusterRootNo; j++)
					if (clH[j] != 0)
					{
						readCluster(clH[j], rootBuf2);
						clEnt = (Entry*)rootBuf2;
						for (ClusterNo k = 0; k < ClusterEntryNo; k++)
							if (freeEntry(clEnt + k))
							{
								//cout << endl << "made new file on entry: " << j << endl;
								writeEntryToBuffer(fname, rootBuf2, clH[i], j);
								return new FileControl(mode, fname, clEnt + k, clH[j], k);
							}
					}
			}
		}
		else
		{
			ClusterNo clNo = newClusterFS(rootCache, rootBuf1, bitVect->getRootPosition(), i);
			if (i < ClusterRootNo / 2)
			{
				clEnt = (Entry*)rootBuf1;
				writeEntryToBuffer(fname, rootBuf1, clNo, 0);
				return new FileControl(mode, fname, clEnt, clNo, 0);
			}
			else
			{
				ClusterNo clNo2 = newClusterFS(rootBuf1, rootBuf2, clNo, 0);
				clEnt = (Entry*)rootBuf2;
				writeEntryToBuffer(fname, rootBuf2, clNo2, 0);
				return new FileControl(mode, fname, clEnt, clNo2, 0);
			}
		}
	}
    return nullptr;
}

FileControl* KernPart::fileControlFromRootDir(char* fname,char mode)
{
    ClusterNo* rootB=(ClusterNo*)rootCache;
    Entry* clEnt;
    ClusterNo* clH;
    char rootBuf1[ClusterSize];
    char rootBuf2[ClusterSize];
	for (ClusterNo i = 0; i < ClusterRootNo; i++) {
		if (rootB[i] != 0)
		{
			readCluster(rootB[i], rootBuf1);
			if (i < ClusterRootNo / 2)
			{
				clEnt = (Entry*)rootBuf1;


				for (ClusterNo j = 0; j < ClusterEntryNo; j++) {
					Entry ent = clEnt[j];
					//cout << "clent+j: " << clEnt + j << endl;
					if ((clEnt + j)->nameEq(fname))
						return new FileControl(mode, fname, clEnt + j, rootB[i], j);
				}
			}
			else
			{
				clH = (ClusterNo*)rootBuf1;
				for (ClusterNo j = 0; j < ClusterRootNo; j++)
					if (clH[j] != 0)
					{
						readCluster(clH[j], rootBuf2);
						clEnt = (Entry*)rootBuf2;
						for (ClusterNo k = 0; k < ClusterEntryNo; k++)
							if ((clEnt + k)->nameEq(fname))
								return new FileControl(mode, fname, clEnt + k, clH[j], k);
					}
			}
		}
	}
    return nullptr;
}

bool KernPart::doesExistRootDir(char* fname)
{
	CriticalSection lck(myPartSem);
	//spojiti sa filecontrolfromrootdir?

    ClusterNo* rootB = (ClusterNo*)rootCache;
    Entry* clEnt;
    ClusterNo* clH;
    char rootBuf1[ClusterSize];
    char rootBuf2[ClusterSize];
    for (ClusterNo i = 0; i < ClusterRootNo; i++)
        if (rootB[i] != 0)
        {
            readCluster(rootB[i], rootBuf1);

			
            if (i < ClusterRootNo / 2)
            {
				//lvl1ind
                clEnt = (Entry*)rootBuf1;
				for (ClusterNo j = 0; j < ClusterEntryNo; j++)
					return ((clEnt+j)->nameEq(fname));
                       
            }
            else
            {	
				//lvl2ind
                clH = (ClusterNo*)rootBuf1;
                for (ClusterNo j = 0; j < ClusterRootNo; j++)
                    if (clH[j] != 0)
                    {
                        readCluster(clH[j], rootBuf2);
                        clEnt = (Entry*)rootBuf2;
                        for (ClusterNo k = 0; k < ClusterEntryNo; k++)
							return ((clEnt + j)->nameEq(fname));
                    }
            }
        }
    return false;
}

File* KernPart::openFile(char *fname, char mode)
{
	CriticalSection  lck(myPartSem);

	//cout << " opening: " << fname << " mode: " << mode << " markeddelete: " << marked << endl;

    if (marked)
        return nullptr;

    map<string, FileControl*>::iterator it = mapOpen.find(fname);
    KernFile* myFile = nullptr;
    if (it != mapOpen.end() && it->second != nullptr) {
		//cout << "found file in map" << endl;
        if (mode == 'r')
        {
            if (it->second->readers == 0 && it->second->writers > 0)
            {	
                it->second->waitReaders++;

				//otkljuca mypartsem lck
				signalAndWait(myPartSem, it->second->readSem);
				wait(myPartSem);
            }
            it->second->readers++;
            return new File(new KernFile(it->second, this, mode));
        }
        if (mode == 'w' || mode == 'a')
        {
            if (it->second->readers > 0 || it->second->writers == 1)
            {
                it->second->waitWriters++;
                signalAndWait(myPartSem, it->second->writeSem);
                wait(myPartSem);
				//wait(it->second->writeSem);
            }
            it->second->writers++;
            return new File(new KernFile(it->second, this, mode));
        }
    }
    

	//if in rootdir
    else if(FileControl* myFileControl = fileControlFromRootDir(fname, mode))
    {
		//cout << "found file in rootdir" << endl;
        mapOpen.insert(pair<char*, FileControl*>(fname, myFileControl));
        return new File(new KernFile(myFileControl, this, mode));
    }

	// make new
    else if(mode=='w')
    {
        FileControl* myFileControl=makeNewFile(fname, mode);
        if(myFileControl)
        {
			mapOpen.insert(pair<char*, FileControl*>(fname, myFileControl));
			return new File(new KernFile(myFileControl, this, mode));
        }
    }

	//failed to find file to read
    return nullptr;
}
    
bool KernPart::deleteFile(char* fname){
	CriticalSection  lck(myPartSem);

    map<string, FileControl*>::iterator it = mapOpen.find(fname);
    if (it->second) {
        mapOpen.erase(fname);
        delete it->second;
        return true;
    }
	return false;
}


bool KernPart::freeEntry(Entry* ent)
{
    for (int i = 0; i < fileNameLen; i++)
        if (ent->name[i]!=0)
            return false;
    return true;
}

void KernPart::writeEntryToBuffer(char* fname, char* buffer, ClusterNo pos, ClusterNo off)
{
    int  k = 0;
    bool flag=false;
    Entry* entry=(Entry*)buffer;
	//cout << "writing fajl" << fname << endl;
    for (unsigned i = 0; i < fileNameLen; i++)
    {
        if (i<strlen(fname) && fname[i]=='.')
        {
            flag = true;
            k=i+1;
        }
        if(!flag)
            entry[off].name[i]=fname[i];
        if(flag)
            entry[off].name[i]=' ';
    }

    flag=false;
    for (size_t i=0; i < fileExtLen; i++)
        if (k >= strlen(fname))
            entry[off].ext[i]=' ';
        else
            entry[off].ext[i]=fname[k++];

    entry[off].indexCluster=bitVect->getFirstEmpty();

    entry[off].size=0;
	//cout << "wrote " << entry[off].name << " ext " << entry[off].ext << " in entry " << off << "INDEXCLUSTER:" << entry[off].indexCluster << endl;
    writeCluster(pos, buffer);
}

ClusterNo KernPart::newClusterFS(char* bufferRoot, char* buffer, ClusterNo pos, ClusterNo off)
{
    ClusterNo k=bitVect->getFirstEmpty();

    ((ClusterNo*) bufferRoot)[off]=k;
    writeCluster(pos, bufferRoot);
    for(ClusterNo i=0; i<ClusterSize; i++)
        buffer[i]=0;
    return k;
}


char KernPart::readRootDir()
{
	CriticalSection lck(myPartSem);

    ClusterNo* rootB=(ClusterNo*)rootCache;
    Entry* clEnt;
    ClusterNo* clH;
    char rootBuf1[ClusterSize];
    char rootBuf2[ClusterSize];
    EntryNum cnt=0;
    // EntryNum end=n+64;
    for(ClusterNo i=0; i<ClusterRootNo;i++)
        if(rootB[i]!=0)
        {
            readCluster(rootB[i], rootBuf1);
            if(i<ClusterRootNo/2)
            {
                clEnt=(Entry*)rootBuf1;
                for(ClusterNo j=0; j<ClusterEntryNo;j++)
                    if(!freeEntry(clEnt+j))
                        cnt++;

            }
            else
            {
                clH=(ClusterNo*)rootBuf1;
                for(ClusterNo j=0; j<ClusterRootNo;j++)
                    if(clH[j]!=0)
                    {
                        readCluster(clH[j], rootBuf2);
                        clEnt=(Entry*)rootBuf2;
                        for(ClusterNo k=0; k<ClusterEntryNo;k++)
                            if(!freeEntry(clEnt+k))
                                cnt++;
                    }
            }
        }
    return (char)cnt;
}
// void KernPart::saveandclose() {}

 ClusterNo KernPart::getNewEmpty() {
     return bitVect->getFirstEmpty();
 }

 void KernPart::freeCluster(ClusterNo clno) {
     bitVect->free(clno);
 }

 KernPart::~KernPart() {
     writeCluster(bitVect->getRootPosition(), rootCache);
     delete bitVect;
 }