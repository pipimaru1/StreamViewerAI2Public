#pragma once
#include <iostream>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sstream>
#include <fstream>

//とりあえず・・・
struct AiRecordHeader
{
    std::wstring Category;
    std::wstring Location;
    std::wstring StreamURL;
    std::wstring Timestamp;
};

struct AiSqlOutput
{
    //std::wstring    table;
    long long       EventID;  // INT,
    std::wstring    Category; //NVARCHAR(50),
    std::wstring    Location; // NVARCHAR(255),
    std::wstring    StreamURL;// NVARCHAR(255),
    std::wstring    Timestamp;// DATETIME,
    int             Index_per_frame; // INT,
    int             index_max_frame; //  INT,
    int             idx; //  INT,
    int             ClassID; //  INT,
    float           Confidence; //  FLOAT,
    std::wstring    ClassName; //  NVARCHAR(50),
    float           ScoreThreshold; //  FLOAT,
    float           NmsThreshold; //  FLOAT,
    float           ConfidenceThreshold; //  FLOAT,
    int             x0; //  INT,
    int             y0; //  INT,
    int             x1; //  INT,
    int             y1; //  INT,
    int             Width; //  INT,
    int             Height; //  INT,
    std::wstring    OnnxFileName; //  NVARCHAR(255),
    std::wstring    NamesFileName; //  NVARCHAR(255),
    int             ImageWidth; //  INT,
    int             ImageHeight; //  INT

public:
    AiSqlOutput();
    AiSqlOutput(
        //std::wstring&    table,
        int             _EventID,  // INT,
        std::wstring&   _Category, //NVARCHAR(50),
        std::wstring&   _Location, // NVARCHAR(255),
        std::wstring&   _StreamURL,// NVARCHAR(255),
        std::wstring&   _Timestamp,// DATETIME,
        int             _Index_per_frame, // INT,
        int             _index_max_frame, //  INT,
        int             _idx, //  INT,
        int             _ClassID, //  INT,
        float           _Confidence, //  FLOAT,
        std::wstring&   _ClassName, //  NVARCHAR(50),
        float           _ScoreThreshold, //  FLOAT,
        float           _NmsThreshold, //  FLOAT,
        float           _ConfidenceThreshold, //  FLOAT,
        int             _x0, //  INT,
        int             _y0, //  INT,
        int             _x1, //  INT,
        int             _y1, //  INT,
        int             _Width, //  INT,
        int             _Height, //  INT,
        std::wstring&   _OnnxFileName, //  NVARCHAR(255),
        std::wstring&   _NamesFileName, //  NVARCHAR(255),
        int             _ImageWidth, //  INT,
        int             _ImageHeight //  INT
    );
};
struct SqlServer
{
    std::wstring server_name;
    std::wstring db_name;
    std::wstring uid;
    std::wstring pwd;
    std::wstring table;
    std::wstring image_index_folder;
public:
    SqlServer();
    SqlServer(
        std::wstring& _server_name,
        std::wstring& _db_name,
        std::wstring& _uid,
        std::wstring& _pwd,
        std::wstring& _table,
        std::wstring& _image_index_folder
    );
};

struct SqlImageOutput
{
    std::wstring    StreamURL;// NVARCHAR(20) PRIMARY KEY,
    long long       EventID;// INT,
    std::wstring    Category;// NVARCHAR(50),
    std::wstring    Location; //NVARCHAR(80),
    std::wstring    Timestamp;// DATETIME,
    int             ImageWidth; // INT,
    int             ImageHeight;// INT,
    cv::Mat         image;// VARBINARY(MAX)

    std::vector<int> CompressOption; //cvの圧縮ルーチンに渡すオプション
    std::wstring    CompressType;   //データベースに入れる。8文字。JPG,PNGなど 

};

////////////////////////////////////////////////////////////////////////////////////////////////
int Sql_Write(SqlServer& _sql_server, std::vector<AiSqlOutput>& _aop_data, int _aop_size);
int Sql_ImageWriteS(SqlServer& _sql_server, SqlImageOutput& _sql_image_data);
int Sql_ImageWriteW(SqlServer& _sql_server, SqlImageOutput& _sql_image_data);
int Sql_ImageWriteChunksW(SqlServer& _sql_server, SqlImageOutput& _sql_image_data);
