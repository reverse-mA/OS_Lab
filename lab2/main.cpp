#include <iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <vector>
#include<string>
using namespace std;
//一个字节
typedef unsigned char Byte1;
//两个字节
typedef unsigned short Byte2;
//四个字节
typedef unsigned int Byte4;
//BPB，引导扇区存的数据，从第11位开始，总长度为25位

//错误Img文件提示
string hintImg = "Not a valid FAT12 img.";

//错误指令提示
string wrongCommand = "Not a valid command, try ls||ls -l||cat||exit";

//不存在的文件
string wrongFile = "Not a exist file, try another";

//请求参数是一个目录，不支持cat
string wrongPara = "Not a file, try another";

//请求参数是一个文件，不支持ls、ls -l
string wrongPara1 = "Not a dir, try another";

char *wrong1;

char *wrong2;

char *wrong3;

char *wrong4;

char *wrong5;

int BytsPerSec;

int SecPerClus;

int RsvdSecCnt;

int NumFATS;

int RootEntCnt;

int FATSz16;

struct Dir{
    bool flag;
    char name[11];
    int childDir;
    int childDocument;
};

struct Document{
    bool flag;
    char name[11];
    int size;
    int cluster;//把文件的簇号存着
};

union Matter
{
    //true为目录，false为普通文件
    bool flag;
    Dir dir;
    Document document;
};

//可以用union
struct Node{
    Matter matter;
    vector<Node>nodes;
};
#pragma pack (1) /*指定按1字节对齐*/
struct BPB{
    //每扇区字节数
    Byte2 BytsPerSec;

    //每簇占用的扇区数
    Byte1 SecPerClus;

    //boot占用的扇区数
    Byte2 RsvdSecCnt;

    //FAT表的记录数
    Byte1 NumFATS;

    //最大根目录文件数
    Byte2 RootEntCnt;

    //逻辑扇区总数
    Byte2 TotSec16;

    //媒体描述符
    Byte1 Media;

    //每个FAT占用扇区数
    Byte2 FATSz16;

    //每个磁道扇区数
    Byte2 SecPerTrk;

    //磁头数
    Byte2 NumHeads;

    //隐藏扇区数
    Byte4 HiddSec;

    //如果BPB_TotSec16是0，则在这里记录扇区总数
    Byte4 TotSec32;
    
};
//根目录
struct RootDir{

    //前8位文件名，后三位扩展名
    char DIR_NAME[11];

    //文件属性
    Byte1 DIR_ATTR;

    //保留位
    char Reserved[10];

    //最后一次写入时间
    Byte2 DIR_WrtTime;

    //最后一次写入日期
    Byte2 DIR_WrtDate;

    //次条目对应的开始簇号
    Byte2 DIR_FstClus;

    //文件大小
    Byte4 DIR_FileSize;
};

#pragma pack () /*取消指定对齐，恢复缺省对齐*/

extern "C" {  
    void print(char*);
    void printACluster(char*);
    void printRed();
    void printBlack();
    void printLineFeed();
    void printSpace();
    void printColon();
    void printPoint();
    void iprint(int);
}  

bool isDirNameValid(const char *dirName){
    bool flag=false;
    for(int i=0;i<10;i++){
        if(*(dirName+i)!=0&&*(dirName+i)>='A'&&*(dirName+i)<='Z'){
            flag=true;
            break;
        }
    }
    return flag;
}

bool isSubDirValid(const char *dirName){
    bool flag=false;
    for(int i=0;i<10;i++){
        if(*(dirName+i)!=' '&&*(dirName+i)>='A'&&*(dirName+i)<='Z'){
            flag=true;
            break;
        }
    }
    //if(*(dirName+8)==0&&*(dirName+9)==0&&*(dirName+10)==0)flag=false;
    return flag;
}

bool isDirNotNull(RootDir &dir){
    bool flag=false;
    char*dirName = dir.DIR_NAME;
    for(int i=0;i<10;i++){
        if(*(dirName+i)!=0){
            flag=true;
            break;
        }
    }
    if(dir.DIR_ATTR!=0)flag=true;
    char*reserve = dir.Reserved;
    for(int i=0;i<9;i++){
        if(*(reserve+i)!=0){
            flag=true;
            break;
        }
    }
    if(dir.DIR_WrtTime!=0)flag=true;
    if(dir.DIR_WrtDate!=0)flag=true;
    if(dir.DIR_FileSize!=0)flag=true;
    if(dir.DIR_FstClus!=0)flag=true;
    return flag;
}
bool read2BPB(FILE* fat12,BPB *bpbPointer){
    int checkPoint1,checkPoint2;
    //从第11位开始是bpb
    checkPoint1=fseek(fat12,11,SEEK_SET);
    //开能否往后读25位,1字节为单位，读25位
    checkPoint2=fread(bpbPointer,1,sizeof(BPB),fat12);
    return checkPoint1==0&&checkPoint2==sizeof(BPB);
}
bool read2RootDir(vector<RootDir>&dirs, FILE* fat12){
    int checkPoint1,checkPoint2;
    //先移到根目录的起始地址
    int offset = (RsvdSecCnt+(NumFATS*FATSz16))*BytsPerSec;
    checkPoint1=fseek(fat12,offset,SEEK_SET);
    if(checkPoint1!=0)return false;
    //循环读取文件到vector里
    for(int i=0;i<RootEntCnt;i++){
        RootDir *dir = new RootDir;
        checkPoint2=fread(dir,1,sizeof(RootDir),fat12);
        if(checkPoint2!=sizeof(RootDir)||checkPoint1!=0)return false;
        if(isDirNameValid(dir->DIR_NAME)){
            dirs.push_back(*dir);
            delete dir;
        }
        else if(isDirNotNull(*dir)){
        	delete dir;
        }
        else{
        	delete dir;
        	break;
        }
        
    }
    return true;
}

void processName(char *name){
     if(name[8]!=' '){
            int pointer1=0;
            int pointer2=8;
            while(name[pointer1]!=' ')pointer1++;
            name[pointer1]='.';
            pointer1++;
            while(pointer2<11){
                name[pointer1]=name[pointer2];
                pointer1++;
                pointer2++;
            }
            while(pointer1<11){
                name[pointer1]='\0';
                pointer1++;
            }
        }
}

void processRootDir(vector<RootDir>&dirs){
    for(int i=0;i<dirs.size();i++){
        if(dirs[i].DIR_NAME[8]!=' '){
            processName(dirs[i].DIR_NAME);
        }
    }
}

//处理Matter，把文件夹名规格化
void processMatter(vector<Matter>&matters){
    for(int i=0;i<matters.size();i++){
        if(matters[i].flag){
            for(int k=0;k<11;k++){
                if(matters[i].dir.name[k]==' '){
                    matters[i].dir.name[k]='\0';
                }
            }
        }
    }
}

int getNextCluster(FILE* fat12,int currentCluster){
	//把boot区跳过去
	int checkPoint1;
	int offset = RsvdSecCnt*BytsPerSec+currentCluster*3/2;
    int type = currentCluster%2;
	checkPoint1=fseek(fat12,offset,SEEK_SET);
	Byte1 nextCluster[2];
	fread(nextCluster,1,2,fat12);
    if (type == 0)return Byte2(((nextCluster[1]&0x0F) << 8)|nextCluster[0]);
    else return Byte2((nextCluster[1] << 4)|((nextCluster[0] >> 4)&0x0F));

}

bool findDir(vector<RootDir>&dirs,FILE* fat12, vector<Matter> &files, string fileName){  
    //递归终止条件是dirs的size是0，不用特判
    int checkPoint1,checkPoint2;
    //数据区
    int offset = (RsvdSecCnt+(NumFATS*FATSz16))*BytsPerSec+RootEntCnt*sizeof(RootDir);
    //int offset=bpb.BytsPerSec * (bpb.RsvdSecCnt + bpb.FATSz16*bpb.NumFATS + (bpb.RootEntCnt*32 + bpb.BytsPerSec - 1)/bpb.BytsPerSec );
    checkPoint1=fseek(fat12,offset,SEEK_SET);
    if(checkPoint1!=0)return false;
    //先把该目录读进去
    Matter *matter = new Matter;
    matter->flag = true;
    //读名字有点麻烦
    for(int i=0;i<11;i++){
        matter->dir.name[i] = fileName[i];
    }
    matter->dir.childDir = 0;
    matter->dir.childDocument = 0;
    for(int i=0;i<dirs.size();i++){    
        if(dirs[i].DIR_ATTR==16){
            matter->dir.childDir+=1;
        }
        else if(dirs[i].DIR_ATTR==32||dirs[i].DIR_ATTR==0){
            matter->dir.childDocument+=1;
        }
    }
    files.push_back(*matter);
    //如果发现他是空目录，直接返回
    if(matter->dir.childDir == 0&&matter->dir.childDocument==0){
        return true;
    }
    for(int i=0;i<dirs.size();i++){
        //再看他的子目录和子文件
        vector<RootDir>subDirs;
        //子文件夹
        if(dirs[i].DIR_ATTR==16){
            //跳过.和..
            int num = offset+(dirs[i].DIR_FstClus-2)*SecPerClus*BytsPerSec+sizeof(RootDir)*2;
            checkPoint1=fseek(fat12,num,SEEK_SET);
            if(checkPoint1!=0)return false;
            for(int k=0;k<BytsPerSec/sizeof(RootDir);k++){
                RootDir *dir = new RootDir;
                checkPoint2=fread(dir,1,sizeof(RootDir),fat12);
                if(isSubDirValid(dir->DIR_NAME)){
                    subDirs.push_back(*dir);
                    delete dir;
                }
                else {
                    findDir(subDirs,fat12,files,dirs[i].DIR_NAME);
                    break;
                }
            }
        }
        //子文件
        else if(dirs[i].DIR_ATTR==32||dirs[i].DIR_ATTR==0){
            Matter *matter1 = new Matter;
            matter1->flag=false;
            processName(dirs[i].DIR_NAME);
            memset(matter1->document.name,'\0',11);
            for(int k=0;k<11;k++){
                matter1->document.name[k] = dirs[i].DIR_NAME[k];
            }
            matter1->document.size = dirs[i].DIR_FileSize;
            matter1->document.cluster = dirs[i].DIR_FstClus;
            files.push_back(*matter1);
        }
    }
    return true;
}



void split(const string& s,vector<string>& sv,const char flag = ' ') {
    sv.clear();
    string temp;
    for(int i=0;i<s.size();i++){
        if(s[i]!=flag)temp+=s[i];
        else{
            if(temp.size()!=0)sv.push_back(temp);
            temp="";
        }
    }
    if(temp.size()!=0)sv.push_back(temp);
    return;
}

bool isInputValid(const string &command, vector<string> &list){
    if(command.substr(0,2)!="ls"&&command.substr(0,3)!="cat"&&command!="exit"){
        return false;
    }
    if(command=="exit")return true;
    if(command.substr(0,2)=="ls"){
        if(command!="ls"&&command[2]!=' ')return false;
        else{
            split(command,list);
            int res=0;
            for(int i=1;i<list.size();i++){
                if(list[i][0]!='-'){
                    res+=1;
                    if(res>1)return false;
                }
                else{
                    for(int k=1;k<list[i].size();k++){
                        if(list[i][k]!='l')return false;
                    }
                }
            }
        }
    }
    if(command.substr(0,3)=="cat"){
        if(command[3]!=' ')return false;
        else{
            split(command,list);
            if(list.size()>2)return false;
        }
    }
    return true;
}
//这里的节点一定是一个文件目录
int getStart(Node* node){
	int res=node->matter.dir.childDir+node->matter.dir.childDocument;
	for(int i=0;i<node->nodes.size();i++){
		if(node->nodes[i].matter.flag){
			res+=getStart(&node->nodes[i]);
		}
	}
	return res;
}
void makeTree(const vector<Matter> &files,int num,Node* node,int start){
    //if(start>=files.size())return;
    for(int i=0;i<num;i++){
        //普通文件
        Node* subNode = new Node;
        subNode->matter=files[start]; 
        if(files[start].flag){
           makeTree(files,files[start].dir.childDir+files[start].dir.childDocument,subNode,start+1);
           start = start+getStart(subNode)+1;        
        }else{
        	start+=1;
        }
        node->nodes.push_back(*subNode);
    }
}

//获取一棵树的根节点
Node* getTree(const vector<Matter> &files){
    Node* root = new Node;
    root->matter = files[0];
    makeTree(files,files[0].dir.childDir+files[0].dir.childDocument,root,1);
    return root;
}

bool compareS(string s1,const char* const s2){
    for(int i=0;s1[i]!='\0'||s2[i]!='\0';i++){
        if(s1[i]!=s2[i])return false;
    }
    return true;
}

//查询以root为根节点的树上有没有path指定的节点
Node getNode(Node *node,string path){
    if(path[0]=='/')path = path.substr(1,path.size());
    vector<string>paths;
    split(path,paths,'/');
    Node *temp = new Node;
    memcpy(temp,node,sizeof(Node));
    for(int i=0;i<paths.size();i++){
        for(int k=0;k<temp->nodes.size();k++){
            if(compareS(paths[i],temp->nodes[k].matter.dir.name)){
                if(i==paths.size()-1){
                    return temp->nodes[k];
                }
                memcpy(temp,&temp->nodes[k],sizeof(Node));
                break;
            }
        }
    }
    return *node;
}

bool isPathExist(Node* node,string path){
    int start=0;
    while(path[start]=='/')start++;
    path = path.substr(start,path.size());
    if(path.size()==0)return true;
    vector<string>paths;
    split(path,paths,'/');
    Node *temp = new Node;
    memcpy(temp,node,sizeof(Node));
    for(int i=0;i<paths.size();i++){
        for(int k=0;k<temp->nodes.size();k++){
            if(compareS(paths[i],temp->nodes[k].matter.dir.name)){
                memcpy(temp,&temp->nodes[k],sizeof(Node));
                if(i==paths.size()-1){
                    return true;
                }
                break;
            }
        }
    }
    return false;
}
char* string2charArray(string str){
    int len = str.size();
    char *p = (char*)malloc(sizeof(char)*(len+1));
    for(int i=0;i<len;i++){
        p[i]=str[i];
    }
    p[len] = '\0';
    return p;
}
void printTree(string path,Node root){
    if(root.matter.flag){
        char *p = string2charArray(path);
        print(p);
        printColon();
        printLineFeed();
        if(path!="/"){
            printRed();
            printPoint();
            printSpace();
            printPoint();
            printPoint();
            printSpace();
            printBlack();
        }
        for(int i=0;i<root.nodes.size();i++){
            if(root.nodes[i].matter.flag){
                printRed();
            }
            else{
                printBlack();   
            }
            print(root.nodes[i].matter.dir.name);
            if(i!=root.nodes.size()-1)printSpace();
        }
        printLineFeed();
        printBlack();
        for(int i=0;i<root.nodes.size();i++){
            if(root.nodes[i].matter.flag)printTree(path+root.nodes[i].matter.dir.name+"/",root.nodes[i]);
        }
    }
    else{
        print(wrong5);
        printLineFeed();
    }
}

void printTreeL(string path,Node root){
    if(root.matter.flag){
        char *p = string2charArray(path);
        print(p);
        printSpace();
        iprint(root.matter.dir.childDir);
        printSpace();
        iprint(root.matter.dir.childDocument);
        printColon();
        printLineFeed();
        if(path!="/"){
            printRed();
            printPoint();
            printLineFeed();
            printPoint();
            printPoint();
            printLineFeed();
            printBlack();
        }
        for(int i=0;i<root.nodes.size();i++){
            if(root.nodes[i].matter.flag){
                printRed();
            }
            else{
                printBlack();   
            }
            print(root.nodes[i].matter.dir.name);
            printSpace();
            printSpace();
            printBlack();
            iprint(root.nodes[i].matter.dir.childDir);
            if(root.nodes[i].matter.flag){
                printSpace();
                iprint(root.nodes[i].matter.dir.childDocument);
            }
            printLineFeed();
            printBlack();
        }
        printLineFeed();
        for(int i=0;i<root.nodes.size();i++){
            if(root.nodes[i].matter.flag)printTreeL(path+root.nodes[i].matter.dir.name+"/",root.nodes[i]);
        }
    }
    else{
        print(wrong5);
        printLineFeed();
    }
}

void input(Node*root, FILE* fat12){
    string command;
    vector<string>list;
    do
    {
        string mark = ">";
        char *mark1 = string2charArray(mark);
        print(mark1);
        getline(cin,command);
        if(command=="exit")break;
        if(isInputValid(command,list)){
            if(list[0]=="ls"){
                string path;
                //看有没有-l参数
                bool flag = false;
                for(int i=1;i<list.size();i++){
                    if(list[i][0]!='-'){
                        path = list[i];
                    }
                    else{
                        flag=true;
                    }
                }
                //ls
                if(!isPathExist(root,path)){
                    print(wrong3);
                    printLineFeed();
                    continue;
                }
                Node temp = getNode(root,path);
                //for(int i=0;i<temp.nodes.size();i++){}
                if(path.size()==0||path[path.size()-1]!='/')path+="/";
                if(path[0]!='/')path="/"+path;
                if(!flag){
                    printTree(path,temp);
                }
                //ls -l
                else{
                    printTreeL(path,temp);
                }
            }
            else if(list[0]=="cat"){
                int checkPoint1, checkPoint2;
                string path = list[1];
                if(!isPathExist(root,path)){
                    print(wrong3);
                    printLineFeed();
                    continue;
                }
                Node temp = getNode(root,path);
                if(temp.matter.flag){
                    print(wrong4);
                    printLineFeed();
                    continue;
                }
                int size = temp.matter.document.size;
                int cluster = temp.matter.document.cluster;
                while(true){
                    int offset = (RsvdSecCnt+(NumFATS*FATSz16))*BytsPerSec+RootEntCnt*sizeof(RootDir)+(cluster-2)*SecPerClus*BytsPerSec;
                    checkPoint1 = fseek(fat12,offset,SEEK_SET);
                    if(size<=512)break;
                    else{
                        char*file = (char*)malloc(sizeof(char)*512);
                        checkPoint2 = fread(file,1,512,fat12);
                        printACluster(file);
                        size-=512;
                        cluster = getNextCluster(fat12,cluster);
                    }   
                }    
                if(checkPoint1!=0){
                    print(wrong1);
                    printLineFeed();
                    continue;
                }
                char*file = (char*)malloc(sizeof(char)*size+1);
                checkPoint2 = fread(file,1,size+1,fat12);
                print(file);
                printLineFeed();
                free(file);
            }
        }
        else{
            print(wrong2);
            printLineFeed();
        }
    } while (command!="exit");
    
}

int main(){
    FILE* fat12;
    fat12 = fopen("a.img","rb");	//以二进制只读的形式打开文件，返回的是一个指针
    wrong1 = string2charArray(hintImg);
    wrong2 = string2charArray(wrongCommand);
    wrong3 = string2charArray(wrongFile);
    wrong4 = string2charArray(wrongPara);
    wrong5 = string2charArray(wrongPara1);
    BPB bpb;
    BPB *bpbPointer = &bpb;
    bool flag;
    flag = read2BPB(fat12,bpbPointer);
    BytsPerSec = bpb.BytsPerSec;
    SecPerClus = bpb.SecPerClus;
    RsvdSecCnt = bpb.RsvdSecCnt;
    NumFATS = bpb.NumFATS;
    RootEntCnt = bpb.RootEntCnt;
    FATSz16 = bpb.FATSz16;
    if(!flag){
        print(wrong1);
        printLineFeed;
        return 0;
    }
    vector<RootDir>dirs;
    //存放所有的文件夹和文件
    vector<Matter>files;
    flag=read2RootDir(dirs, fat12);
    if(!flag){
        print(wrong1);
        printLineFeed;
        return 0;
    }
    flag=findDir(dirs, fat12, files,"/");
    processMatter(files);
    if(!flag){
        print(wrong1);
        printLineFeed;
        return 0;
    }
    Node* root = getTree(files);
    input(root,fat12);
    fclose(fat12);
    return 0;
}
