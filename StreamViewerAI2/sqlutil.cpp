#include "stdafx.h"
#include "sqlutil.h"
#include "logfile.h"
#include "mylib.h"
#include "base64.hpp"

#ifdef _DEBUG
#define _DEBUG_SQL
#else
#undef _DEBUG_SQL
#endif

//#define _DEBUG_SQL

////////////////////////////////////////////////////////////////////////////////////////////////
//�R���X�g���N�^ �����l�̓T���v���p
AiSqlOutput::AiSqlOutput()
{
    EventID = 2;
    Category = L"Cat"; //NVARCHAR
    Location = L"Loc"; //NVARCHA
    StreamURL = L"URL";// NVARCHA
    Timestamp = L"2024-04-10 00:00:13";// DATETIM
    Index_per_frame = 7; // 
    index_max_frame = 8; // 
    idx = 9; //  INT,
    ClassID = 10; //  INT,
    Confidence = 10.1f; //  FLOA
    ClassName = L"class"; //  NVARC
    ScoreThreshold = 13.1f; //  
    NmsThreshold = 13.2f; //  FL
    ConfidenceThreshold = 13.3f;
    x0 = 141; //  INT,
    y0 = 142; //  INT,
    x1 = 143; //  INT,
    y1 = 144; //  INT,
    Width = 145; //  INT,
    Height = 146; // INT,
    OnnxFileName = L"onnx"; //  NV
    NamesFileName = L"name"; //  N
    ImageWidth = 17; //  INT,
    ImageHeight = 1080; //  INT
}

////////////////////////////////////////////////////////////////////////////////////////////////
SqlServer::SqlServer()
{
    server_name = L"";
    db_name = L"";
    uid = L"";
    pwd = L"";
}

SqlServer::SqlServer(
    std::wstring& _server_name,
    std::wstring& _db_name,
    std::wstring& _uid,
    std::wstring& _pwd,
    std::wstring& _table,
    std::wstring& _image_index_folder
)
{
    server_name = _server_name;
    db_name = _db_name;
    uid = _uid;
    pwd = _pwd;
    table = _table;
    image_index_folder = _image_index_folder;
}

inline void SQL_ITEM_HED(std::wostringstream& _wostr, const wchar_t* _db_name)
{
    _wostr << L"INSERT INTO " << _db_name << L" (";
}

inline void SQL_ITEM(std::wostringstream& _wostr, const wchar_t* _value)
{
    _wostr << _value << L",";
}

inline void SQL_VALU_BINT(std::wostringstream& _wostr, long long _value)
{
    _wostr << _value << L",";
}

inline void SQL_VALU_INT(std::wostringstream& _wostr, int _value)
{
    _wostr << _value << L",";
}
inline void SQL_VALU_FLT(std::wostringstream& _wostr, float _value)
{
    _wostr << _value << L",";
}
inline void SQL_VALU_CHR(std::wostringstream& _wostr, const wchar_t* _value)
{
    _wostr << L"'" << _value << "',";
}
inline void SQL_VALU_CHR(std::wostringstream& _wostr, std::wstring& _value)
{
    _wostr << L"'" << _value << "',";
}

inline void SQL_ITEM_END(std::wostringstream& _wostr, const wchar_t* _value)
{
    _wostr << _value << L")";
}

inline void SQL_VALU_HED(std::wostringstream& _wostr)
{
    _wostr << L"VALUES";
}

inline void SQL_VALU_STA(std::wostringstream& _wostr)
{
    _wostr << L"(";
}

//���ɃJ���}���t���Ȃ��A�J�b�R�I��肪��
inline void SQL_VALU_INT_END(std::wostringstream& _wostr, int _value)
{
    _wostr << _value << L")";
}

//value set����؂�J���}
inline void SQL_VALU_CMM(std::wostringstream& _wostr)
{
    _wostr << L",";// << std::ends;
}

//�X�e�[�g�����g�̍Ō�̃Z�~�R����
inline void SQL_STAT_END(std::wostringstream& _wostr)
{
    _wostr << L";" << std::ends;
}

AiSqlOutput::AiSqlOutput(
    int             _EventID,  // INT,
    std::wstring&    _Category, //NVARCHAR(50),
    std::wstring&    _Location, // NVARCHAR(255),
    std::wstring&    _StreamURL,// NVARCHAR(255),
    std::wstring&    _Timestamp,// DATETIME,
    int             _Index_per_frame, // INT,
    int             _index_max_frame, //  INT,
    int             _idx, //  INT,
    int             _ClassID, //  INT,
    float           _Confidence, //  FLOAT,
    std::wstring&    _ClassName, //  NVARCHAR(50),
    float           _ScoreThreshold, //  FLOAT,
    float           _NmsThreshold, //  FLOAT,
    float           _ConfidenceThreshold, //  FLOAT,
    int             _x0, //  INT,
    int             _y0, //  INT,
    int             _x1, //  INT,
    int             _y1, //  INT,
    int             _Width, //  INT,
    int             _Height, //  INT,
    std::wstring&    _OnnxFileName, //  NVARCHAR(255),
    std::wstring&    _NamesFileName, //  NVARCHAR(255),
    int             _ImageWidth, //  INT,
    int             _ImageHeight //  INT
)
{
    EventID = _EventID;  // INT,
    Category = _Category; //NVARCHAR(50),
    Location = _Location; // NVARCHAR(255),
    StreamURL = _StreamURL;// NVARCHAR(255),
    Timestamp = _Timestamp;// DATETIME,
    Index_per_frame = _Index_per_frame; // INT,
    index_max_frame = _index_max_frame; //  INT,
    idx = _idx; //  INT,
    ClassID = _ClassID; //  INT,
    Confidence = _Confidence; //  FLOAT,
    ClassName = _ClassName; //  NVARCHAR(50),
    ScoreThreshold = _ScoreThreshold; //  FLOAT,
    NmsThreshold = _NmsThreshold; //  FLOAT,
    ConfidenceThreshold = _ConfidenceThreshold; //  FLOAT,
    x0 = _x0; //  INT,
    y0 = _y0; //  INT,
    x1 = _x1; //  INT,
    y1 = _y1; //  INT,
    Width = _Width; //  INT,
    Height = _Height; //  INT,
    OnnxFileName = _OnnxFileName; //  NVARCHAR(255),
    NamesFileName = _NamesFileName; //  NVARCHAR(255),
    ImageWidth = _ImageWidth; //  INT,
    ImageHeight = _ImageHeight; //  INT
}

//�e�X�g�p�ł͂Ȃ�
int SQL_ITEMS(std::wostringstream& _wostr, std::wstring& _table)
{
    SQL_ITEM_HED(_wostr, _table.c_str());
    SQL_ITEM(_wostr, L"EventID");
    SQL_ITEM(_wostr, L"Category");
    SQL_ITEM(_wostr, L"Location");
    SQL_ITEM(_wostr, L"StreamURL");
    SQL_ITEM(_wostr, L"Timestamp");
    SQL_ITEM(_wostr, L"Index_per_frame");
    SQL_ITEM(_wostr, L"index_max_frame");
    SQL_ITEM(_wostr, L"idx");
    SQL_ITEM(_wostr, L"ClassID");
    SQL_ITEM(_wostr, L"Confidence");
    SQL_ITEM(_wostr, L"ClassName");
    SQL_ITEM(_wostr, L"ScoreThreshold");
    SQL_ITEM(_wostr, L"NmsThreshold");
    SQL_ITEM(_wostr, L"ConfidenceThreshold");
    SQL_ITEM(_wostr, L"x0");
    SQL_ITEM(_wostr, L"y0");
    SQL_ITEM(_wostr, L"x1");
    SQL_ITEM(_wostr, L"y1");
    SQL_ITEM(_wostr, L"Width");
    SQL_ITEM(_wostr, L"Height");
    SQL_ITEM(_wostr, L"OnnxFileName");
    SQL_ITEM(_wostr, L"NamesFileName");
    SQL_ITEM(_wostr, L"ImageWidth");
    SQL_ITEM_END(_wostr, L"ImageHeight");

    //_tempStr = _wostr.str();
    //ss_sql = _tempStr.c_str();
    return 0;
}
int SQL_VALUS(std::wostringstream& _wostr, AiSqlOutput& _ai)
{
    SQL_VALU_STA(_wostr);
    SQL_VALU_BINT(_wostr, _ai.EventID);
    SQL_VALU_CHR(_wostr, _ai.Category);
    SQL_VALU_CHR(_wostr, _ai.Location);
    SQL_VALU_CHR(_wostr, _ai.StreamURL);
    SQL_VALU_CHR(_wostr, _ai.Timestamp);
    SQL_VALU_INT(_wostr, _ai.Index_per_frame);
    SQL_VALU_INT(_wostr, _ai.index_max_frame);
    SQL_VALU_INT(_wostr, _ai.idx);
    SQL_VALU_INT(_wostr, _ai.ClassID);
    SQL_VALU_FLT(_wostr, _ai.Confidence);
    SQL_VALU_CHR(_wostr, _ai.ClassName);
    SQL_VALU_FLT(_wostr, _ai.ScoreThreshold);
    SQL_VALU_FLT(_wostr, _ai.NmsThreshold);
    SQL_VALU_FLT(_wostr, _ai.ConfidenceThreshold);
    SQL_VALU_INT(_wostr, _ai.x0);
    SQL_VALU_INT(_wostr, _ai.y0);
    SQL_VALU_INT(_wostr, _ai.x1);
    SQL_VALU_INT(_wostr, _ai.y1);
    SQL_VALU_INT(_wostr, _ai.Width);
    SQL_VALU_INT(_wostr, _ai.Height);
    SQL_VALU_CHR(_wostr, _ai.OnnxFileName);
    SQL_VALU_CHR(_wostr, _ai.NamesFileName);
    SQL_VALU_INT(_wostr, _ai.ImageWidth);
    SQL_VALU_INT_END(_wostr, _ai.ImageHeight);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////
const SQLWCHAR* make_connectionStringW(SqlServer& _sql_server, std::wstring& _tempStr)
{
    std::wostringstream _wostr;
    const SQLWCHAR* ss_sql;
    //SQLWCHAR connectionString[] = L"DRIVER={SQL Server};SERVER=140.81.145.5\\AWIOTQ01;DATABASE=aicamera;UID=sa;PWD=AWSqlServer01;";

    _wostr << L"DRIVER={SQL Server};SERVER=";
    _wostr << _sql_server.server_name;
    _wostr << L";DATABASE=";
    _wostr << _sql_server.db_name;
    _wostr << L";UID=";
    _wostr << _sql_server.uid;
    _wostr << L";PWD=";
    _wostr << _sql_server.pwd;
    _wostr << L";";
    //�ŐV��ODBC�@�\��L����
    _wostr << L"DataTypeCompatibility=80;";

    _tempStr = _wostr.str();
    ss_sql = _tempStr.c_str();

#ifdef _DEBUG
    LOGMSG2("SQLServer", W2S(_tempStr.c_str()));
#endif // DEBUG

    return ss_sql;
}

////////////////////////////////////////////////////////////////////////////////////////////////
const SQLCHAR* make_connectionStringS(SqlServer& _sql_server, std::string& _tempStr)
{
    std::ostringstream _ostr;
    const SQLCHAR* ss_sql;
    //SQLWCHAR connectionString[] = L"DRIVER={SQL Server};SERVER=140.81.145.5\\AWIOTQ01;DATABASE=aicamera;UID=sa;PWD=AWSqlServer01;";

    _ostr << "DRIVER={SQL Server};SERVER=";
    _ostr << W2S(_sql_server.server_name);
    _ostr << ";DATABASE=";
    _ostr << W2S(_sql_server.db_name);
    _ostr << ";UID=";
    _ostr << W2S(_sql_server.uid);
    _ostr << ";PWD=";
    _ostr << W2S(_sql_server.pwd);
    _ostr << ";";
    //�ŐV��ODBC�@�\��L����
    _ostr << "DataTypeCompatibility=80;";

    _tempStr = _ostr.str();
    ss_sql = (const SQLCHAR*)_tempStr.c_str();
#ifdef _DEBUG
    LOGMSG2("SQLServer", _tempStr.c_str());
#endif // DEBUG
    return ss_sql;
}

////////////////////////////////////////////////////////////////////////////////////////////////
const SQLWCHAR* make_sql_statementW(SqlServer& _sql_server, std::vector<AiSqlOutput>& _ai, int _size, std::wstring& _tempStr)
{
    std::wostringstream _wostr;
    const SQLWCHAR* ss_sql;

    //ofs << "AiSqlOutput::make_sql_statement()" << std::endl;

    SQL_ITEMS(_wostr,_sql_server.table);
    SQL_VALU_HED(_wostr);
    for (int i = 0; i < _size; i++)
    {
        if (i != 0)
            SQL_VALU_CMM(_wostr);
        SQL_VALUS(_wostr, _ai[i]);
    }
    SQL_STAT_END(_wostr);

    _tempStr = _wostr.str();
    ss_sql = _tempStr.c_str();

    return ss_sql;
}

////////////////////////////////////////////////////////////////////////////////////////////////
const SQLWCHAR* make_sql_statementS(SqlServer& _sql_server, std::vector<AiSqlOutput>& _ai, int _size, std::wstring& _tempStr)
{
    std::wostringstream _wostr;
    const SQLWCHAR* ss_sql;

    //ofs << "AiSqlOutput::make_sql_statement()" << std::endl;

    SQL_ITEMS(_wostr, _sql_server.table);
    SQL_VALU_HED(_wostr);
    for (int i = 0; i < _size; i++)
    {
        if (i != 0)
            SQL_VALU_CMM(_wostr);
        SQL_VALUS(_wostr, _ai[i]);
    }
    SQL_STAT_END(_wostr);

    _tempStr = _wostr.str();
    ss_sql = _tempStr.c_str();

    return ss_sql;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int Sql_Write(SqlServer& _sql_server, std::vector<AiSqlOutput>& _aop_data, int _aop_size)
{
    SQLHENV hEnv = NULL;
    SQLHDBC hDbc = NULL;
    SQLHSTMT stmt = NULL;
    SQLRETURN ret;

    // ODBC ����������
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

    // �f�[�^�x�[�X�ɐڑ�
    std::wstring tempStr1;
    SQLRETURN retCode;
    SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    SQLWCHAR* connectionString = (SQLWCHAR*)make_connectionStringW(_sql_server, tempStr1);
    retCode = SQLDriverConnect(hDbc, NULL, connectionString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
    {
        LOGMSG("SQL:Connection successful");
    }
    else 
    {
        //AI�����o�����f�[�^������Ȃ��ƁA�X�e�[�g�����g���ُ�ɂȂ�̂ŃG���[���o���B�Ǝv���B
        LOGMSG("SQL:Connection failed");
#ifdef _DEBUG
        // �G���[���b�Z�[�W���擾
        SQLWCHAR sqlState[1024];
        SQLWCHAR errorMessage[1024];
        SQLINTEGER nativeError;
        SQLSMALLINT textLength;
        SQLGetDiagRec(SQL_HANDLE_DBC, hDbc, 1, sqlState, &nativeError, errorMessage, sizeof(errorMessage) - 1, &textLength);
        LOGMSG2("sqlState", W2S(sqlState));
        LOGMSG2("nativeError", nativeError);
        LOGMSG2("errorMessage", W2S(errorMessage));
#endif
    }

    // SQL �X�e�[�g�����g������
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt);
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // SQL �N�G�������s
    std::wstring tempStr;
    ret = SQLExecDirectW(stmt, (SQLWCHAR*)make_sql_statementW(_sql_server,_aop_data, _aop_size, tempStr), SQL_NTS);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        std::wcout << L"Data inserted successfully." << std::endl;
    }
    else {
        //std::wcout << L"Failed to insert data." << std::endl;
        LOGMSG("SQL:Failed to insert data");

#ifdef _DEBUG
        // �G���[���b�Z�[�W���擾
        SQLWCHAR sqlState[1024];
        SQLWCHAR errorMessage[1024];
        SQLINTEGER nativeError;
        SQLSMALLINT textLength;
        SQLGetDiagRec(SQL_HANDLE_DBC, hDbc, 1, sqlState, &nativeError, errorMessage, sizeof(errorMessage) - 1, &textLength);
        LOGMSG2("sqlState", W2S(sqlState));
        LOGMSG2("errorMessage", W2S(errorMessage));
        LOGMSG2("nativeError", nativeError);
        LOGMSG2("tempStr", W2S(tempStr));
#endif
    }

    // ���\�[�X�����
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    return 0;
}
/////////////////////////////////////////////////////////////////////////////
//�摜
SQLRETURN CheckSQLError(SQLRETURN retCode, SQLHANDLE handle, SQLSMALLINT handleType, const std::wstring& context)
{
#ifdef _DEBUG_SQL
    LOGMSG(W2S(context));
    if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
    {
        LOGMSG("SQL:Connection successful");
        //if (retCode == SQL_SUCCESS_WITH_INFO) 
        //    LOGMSG("SQL:Connection successful with information");
            // �����Œǉ������擾����
    }
    else
    {
        SQLWCHAR sqlState[1024], messageText[1024];
        SQLINTEGER nativeError;
        SQLSMALLINT textLength;
        if (SQLGetDiagRec(handleType, handle, 1, sqlState, &nativeError, messageText, sizeof(messageText) / sizeof(SQLWCHAR), &textLength) == SQL_SUCCESS) {
            LOGMSG(W2S(L"SQL Error in " + context));
            LOGMSG2("messageText", W2S(messageText));
            LOGMSG2("sqlState", W2S(sqlState));
            LOGMSG2("nativeError", nativeError);
            //std::wcerr << L"SQL Error in " << context << L": " << messageText << L" (SQL State: " << sqlState << L", Error Code: " << nativeError << L")" << std::endl;
        }
    }
#endif
    return retCode;
}

SQLWCHAR* GenerateMergeSQL(const wchar_t* tableName)
{
    // SQL����g�ݗ��Ă�
    std::wstring sql = L"MERGE INTO " + std::wstring(tableName) + L" AS target "
        L"USING (SELECT "
        L"? AS StreamURL, "
        L"? AS EventID, "
        L"? AS Category, "
        L"? AS Location, "
        L"? AS Timestamp, "
        L"? AS ImageWidth, "
        L"? AS ImageHeight, "
        L"? AS DataType, "
        L"? AS Image "
        L") AS source "
        L"ON (target.StreamURL = source.StreamURL) "
        L"WHEN MATCHED THEN "
        L"UPDATE SET "
        L"target.EventID = source.EventID, "
        L"target.Category = source.Category, "
        L"target.Location = source.Location, "
        L"target.Timestamp = source.Timestamp, "
        L"target.ImageWidth = source.ImageWidth, "
        L"target.ImageHeight = source.ImageHeight, "
        L"target.DataType = source.DataType, "
        L"target.Image = source.Image "
        L"WHEN NOT MATCHED THEN "
        L"INSERT (StreamURL, EventID, Category, Location, Timestamp, ImageWidth, ImageHeight, DataType, Image) "
        L"VALUES ("
        L"source.StreamURL, "
        L"source.EventID, "
        L"source.Category, "
        L"source.Location, "
        L"source.Timestamp, "
        L"source.ImageWidth, "
        L"source.ImageHeight, "
        L"source.DataType, "
        L"source.Image "
        L");";

    // �������𓮓I�Ɋm�ۂ���SQL�����R�s�[
    SQLWCHAR* dynamicSql = new SQLWCHAR[sql.length() + 1];

#ifdef _DEBUG_SQL
    LOGMSG2("tableName", W2S(tableName));
    LOGMSG2("sql", W2S(sql));
    // std::copy���g�p���Ĉ��S�ɃR�s�[
#endif
    std::copy(sql.begin(), sql.end(), dynamicSql);
    dynamicSql[sql.length()] = L'\0';  // �����I��NUL�I�[

    return dynamicSql;
}

SQLRETURN SQLBindParameter_SString(SQLHSTMT& hStmt, SQLUSMALLINT _num, std::string& _string)
{
    SQLRETURN retCode = 0;
    retCode = SQLBindParameter(hStmt, _num, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, _string.length(), 0, (SQLPOINTER)_string.c_str(), 0, NULL);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLBindParameter_String :" + S2W(_string));
    return retCode;
}

SQLRETURN SQLBindParameter_WString(SQLHSTMT& hStmt, SQLUSMALLINT _num, std::wstring& _wstring)
{
    SQLRETURN retCode=0;
    retCode = SQLBindParameter(hStmt, _num, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, _wstring.length(), 0, (SQLPOINTER)_wstring.c_str(), 0, NULL);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLBindParameter_WString :"+ _wstring);
    return retCode;
}

SQLRETURN SQLBindParameter_Bigint(SQLHSTMT& hStmt, SQLUSMALLINT _num, long long& _data)
{
    //return SQLBindParameter(hStmt, _num, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, _wstring.length(), 0, (SQLPOINTER)_wstring.c_str(), 0, NULL);
    //return SQLBindParameter(hStmt, _num, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_BIGINT, sizeof(_data), 0, &_data, 0, NULL);
    SQLRETURN retCode = 0;
    retCode = SQLBindParameter(hStmt, _num, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_BIGINT, sizeof(_data), 0, &_data, 0, NULL);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLBindParameter_Bigint ");
    return retCode;

}

SQLRETURN SQLBindParameter_Int(SQLHSTMT& hStmt, SQLUSMALLINT _num, int& _data)
{
    //return SQLBindParameter(hStmt, _num, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, sizeof(_data), 0, &_data, 0, NULL);
    SQLRETURN retCode = 0;
    retCode = SQLBindParameter(hStmt, _num, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, sizeof(_data), 0, &_data, 0, NULL);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLBindParameter_Int ");
    return retCode;

}

//�ꉞ�������݂͐����@�f�[�^���s��
SQLRETURN SQLBindParameter_Bin2(SQLHSTMT hStmt, SQLUSMALLINT _num, std::vector<unsigned char> imageData)
{
    SQLLEN ColumnSize           = imageData.size();
    SQLLEN BufferLength         = (SQLLEN)imageData.size(); // �o�C�i���f�[�^�̃T�C�Y�� SQLLEN �^�Ŏw��
    SQLSMALLINT DecimalDigits   = 0;
    SQLRETURN retCode           = 0;
    SQLLEN* StrLen_or_IndPtr  = (SQLLEN*)(&BufferLength);
    //SQLLEN StrLen_or_IndPtr   = SQL_LEN_DATA_AT_EXEC(BufferLength);
    //SQLLEN StrLen_or_IndPtr   = BufferLength;
    //SQLLEN* StrLen_or_IndPtr    = (SQLLEN*)(BufferLength);
    //SQLLEN* StrLen_or_IndPtr    = (SQLLEN*)SQL_LEN_DATA_AT_EXEC(BufferLength);

    retCode = SQLBindParameter(
        hStmt,                      //StatementHandle[����]�X�e�[�g�����g �n���h���B
        _num,                       //ParameterNumber[����]�p�����[�^�[�ԍ��B1 ����n�܂�A��������p�����[�^�[�̏����ŏ��Ԃɕ��בւ��B
        SQL_PARAM_INPUT,            //InputOutputType[����] �p�����[�^�[�̌^�B �ڍׂɂ��ẮA�u�R�����g�v�́uInputOutputType �����v���Q�Ƃ��Ă��������B
        SQL_C_BINARY,               //ValueType[����]�p�����[�^�[�� C �f�[�^�^�B �ڍׂɂ��ẮA�u�R�����g�v�́uValueType �����v���Q�Ƃ��Ă��������B
                                    //SQL_C_BINARY	Sqlchar*	unsigned char *
        SQL_LONGVARBINARY,
        //SQL_VARBINARY,              //ParameterType[����]�p�����[�^�[�� SQL �f�[�^�^�B �ڍׂɂ��ẮA�u�R�����g�v�́uParameterType �����v���Q�Ƃ��Ă��������B
                                    //SQL_VARBINARY	VARBINARY(n)	�ő咷 n �̉ϒ��o�C�i�� �f�[�^�B �ő�l�̓��[�U�[�ɂ���Đݒ肳��܂��B[9]
                                    //SQL_BINARY	BINARY(n)	�Œ蒷 n �̃o�C�i�� �f�[�^�B[9]
                                    //SQL_LONGVARBINARY	LONG VARBINARY	�ϒ��o�C�i�� �f�[�^�B �ő咷�̓f�[�^ �\�[�X�Ɉˑ����܂��B[9]
        ColumnSize,                 //ColumnSize[����]�Ή�����p�����[�^�[ �}�[�J�[�̗�܂��͎��̃T�C�Y�B �ڍׂɂ��ẮA�u�R�����g�v�́uColumnSize �����v���Q�Ƃ��Ă��������B
                                    //�A�v���P�[�V������ 64 �r�b�g�� Windows �I�y���[�e�B���O �V�X�e���Ŏ��s�����ꍇ�́AODBC 64 �r�b�g�����Q�Ƃ��Ă��������B
        DecimalDigits,              //DecimalDigits �o�C�i���̎��͖��������@[����]�Ή�����p�����[�^�[ �}�[�J�[�̗�܂��͎��� 10 �i���B ��T�C�Y�̏ڍׂɂ��ẮA�u��T�C�Y�A 10 �i���A�]���I�N�e�b�g�̒����A����ѕ\���T�C�Y�v���Q�Ƃ��Ă��������B
        imageData.data(),           //ParameterValuePtr[�x������]�p�����[�^�[�̃f�[�^�̃o�b�t�@�[�ւ̃|�C���^�[�B �ڍׂɂ��ẮA�u�R�����g�v�́uParameterValuePtr �����v���Q�Ƃ��Ă��������B
        BufferLength,               //BufferLength[���� / �o��]ParameterValuePtr �o�b�t�@�[�̒���(�o�C�g�P��)�B �ڍׂɂ��ẮA�u�R�����g�v�́uBufferLength �����v���Q�Ƃ��Ă��������B
                                    //�A�v���P�[�V������ 64 �r�b�g �I�y���[�e�B���O �V�X�e���Ŏ��s�����ꍇ�́AODBC 64 �r�b�g�����Q�Ƃ��Ă��������B
        (SQLLEN*)StrLen_or_IndPtr   //StrLen_or_IndPtr[�x������]�p�����[�^�[�̒����̃o�b�t�@�[�ւ̃|�C���^�[�B �ڍׂɂ��ẮA�u�R�����g�v�́uStrLen_or_IndPtr �����v���Q�Ƃ��Ă��������B
    );

#ifdef _DEBUG_SQL
    LOGMSG2("hStmt", hStmt);
    LOGMSG2("_num", _num);
    LOGMSG2("ColumnSize", ColumnSize);
    LOGMSG2("BufferLength", BufferLength);
    LOGMSG2("StrLen_or_IndPtr", StrLen_or_IndPtr);
    LOGMSG2("&StrLen_or_IndPtr", &StrLen_or_IndPtr);
    //saveImageAsHtml(image_base64, "chk.html");
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLBindParameter_Bin ");
#endif

    return retCode;
    
    //https://stackoverflow.com/questions/14860330/sqlbindparameter-with-sql-varbinarymax-gives-invalid-precision-value
    //SQLLEN len1 = 0;
    //SQLLEN nThisLen = (SQLLEN)sData.size();
    //SQLBindParameter(handle, (SQLUSMALLINT)4, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_VARBINARY, len1, 0, (SQLCHAR*)&sData.c_str(), nThisLen, &nThisLen);
}
//�ꉞ�������݂͐����@�f�[�^���s��
SQLRETURN SQLBindParameter_Bin(SQLHSTMT hStmt, SQLUSMALLINT _num, std::vector<unsigned char>& imageData)
{
    SQLRETURN retCode = 0;

    SQLLEN ColumnSize = 512*1024;
    SQLLEN BufferLength = (SQLLEN)imageData.size(); // �o�C�i���f�[�^�̃T�C�Y�� SQLLEN �^�Ŏw��
    SQLSMALLINT DecimalDigits = 0;
    //SQLLEN StrLen_or_IndPtr = BufferLength;
    SQLLEN StrLen_or_IndPtr = NULL;

    retCode = SQLBindParameter(
        hStmt,
        _num,
        SQL_PARAM_INPUT,
        SQL_C_BINARY,               
        SQL_LONGVARBINARY,
        ColumnSize,
        DecimalDigits,             
        imageData.data(),
        BufferLength,
        (SQLLEN*)StrLen_or_IndPtr
    );

#ifdef _DEBUG_SQL
    LOGMSG2("hStmt", hStmt);
    LOGMSG2("_num", _num);
    LOGMSG2("ColumnSize", ColumnSize);
    LOGMSG2("BufferLength", BufferLength);
    LOGMSG2("StrLen_or_IndPtr", StrLen_or_IndPtr);
    LOGMSG2("&StrLen_or_IndPtr", &StrLen_or_IndPtr);
    //saveImageAsHtml(image_base64, "chk.html");
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLBindParameter_Bin ");
#endif

    return retCode;
}

#define JPG_COMPRESS 75
//�����̃R�[�h�̓o�O������BimageData�����̃u���b�N�ŏ����āA�������ݎ��ɂ͏����Ă���̂Ńf�[�^����ɂȂ�B
SQLRETURN SQLBindParameter_Mat(SQLHSTMT hStmt, SQLUSMALLINT _num,  cv::Mat& image)
{
    // JPG�t�H�[�}�b�g�ɃG���R�[�h
    std::vector<uchar> imageData;
    std::vector<int> compression_params = { cv::IMWRITE_JPEG_QUALITY, JPG_COMPRESS };  // 90%�̕i���ň��k
    cv::imencode(".jpg", image, imageData, compression_params);

    SQLRETURN retCode = 0;
    SQLLEN ColumnSize = 512 * 1024;
    SQLLEN BufferLength = (SQLLEN)imageData.size(); // �o�C�i���f�[�^�̃T�C�Y�� SQLLEN �^�Ŏw��
    SQLSMALLINT DecimalDigits = 0;
    //SQLLEN StrLen_or_IndPtr = BufferLength;
    SQLLEN StrLen_or_IndPtr = NULL;

    retCode = SQLBindParameter(
        hStmt,
        _num,
        SQL_PARAM_INPUT,
        SQL_C_BINARY,
        SQL_LONGVARBINARY,
        ColumnSize,
        DecimalDigits,
        imageData.data(),
        BufferLength,
        (SQLLEN*)StrLen_or_IndPtr
    );

#ifdef _DEBUG_SQL
    LOGMSG2("hStmt", hStmt);
    LOGMSG2("_num", _num);
    LOGMSG2("ColumnSize", ColumnSize);
    LOGMSG2("BufferLength", BufferLength);
    LOGMSG2("StrLen_or_IndPtr", StrLen_or_IndPtr);
    LOGMSG2("&StrLen_or_IndPtr", &StrLen_or_IndPtr);
    //saveImageAsHtml(image_base64, "chk.html");
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLBindParameter_Mat");
#endif
    return retCode;
}

//https://stackoverflow.com/questions/14860330/sqlbindparameter-with-sql-varbinarymax-gives-invalid-precision-value
//SQLLEN len1 = 0;
//SQLLEN nThisLen = (SQLLEN)sData.size();
//SQLBindParameter(handle, (SQLUSMALLINT)4, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_VARBINARY, len1, 0, (SQLCHAR*)&sData.c_str(), nThisLen, &nThisLen);

#define CHK_JPG_BASE64 FALSE

//SQLExecute�܂Ńf�[�^��ۑ�����K�v������̂ŃO���[�o���ϐ� ���X���b�h�Z�[�t�ł͂Ȃ��̂Œ���
volatile SQLLEN StrLen_or_IndPtr;

//string�� base64�Ȃ�Ascii���������Ȃ̂ŏ�肭�����͂�?
SQLRETURN SQLBindParameter_Mat_b64s(SQLHSTMT hStmt, SQLUSMALLINT _num, cv::Mat& image, std::string& image_base64)
{
    // JPG�t�H�[�}�b�g�ɃG���R�[�h
    std::vector<uchar> _JpgData;
    std::vector<int> compression_params = { cv::IMWRITE_JPEG_QUALITY, JPG_COMPRESS };  // 90%�̕i���ň��k
    cv::imencode(".jpg", image, _JpgData, compression_params);
    algorithm::encode_base64(_JpgData, image_base64); // base64 �G���R�[�h
    //JPG�f�[�^�̃`�F�b�N�R�[�h
    if (CHK_JPG_BASE64)
    {
        saveImageDataToFile(_JpgData, "chk.jpg");
        saveImageAsHtml(image_base64, "chk.html");
    }
    SQLRETURN retCode = 0;
    //SQLLEN ColumnSize = 10 * 1024 * 1024;
    SQLLEN ColumnSize = 2147483647;
    
    SQLLEN BufferLength = (SQLLEN)image_base64.size()*sizeof(char); // �o�C�i���f�[�^�̃T�C�Y�� SQLLEN �^�Ŏw��
    SQLSMALLINT DecimalDigits = 0;
    StrLen_or_IndPtr = BufferLength;
    retCode = SQLBindParameter(
        hStmt,
        _num,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_LONGVARCHAR,
        ColumnSize,
        DecimalDigits,
        (void*)image_base64.c_str(),
        BufferLength * sizeof(wchar_t),
        (SQLLEN*)&StrLen_or_IndPtr
        //NULL
    );
#ifdef _DEBUG_SQL
    LOGMSG2("hStmt", hStmt);
    LOGMSG2("_num", _num);
    LOGMSG2("ColumnSize", ColumnSize);
    LOGMSG2("BufferLength", BufferLength);
    LOGMSG2("StrLen_or_IndPtr", StrLen_or_IndPtr);
    LOGMSG2("&StrLen_or_IndPtr", &StrLen_or_IndPtr);
    LOGMSG2("imageData.size()", image_base64.size());
    //saveImageAsHtml(image_base64, "chk.html");
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLBindParameter_Mat_b64");
#endif
    return retCode;
}

SQLRETURN SQLBindParameter_Mat_b64w(SQLHSTMT hStmt, SQLUSMALLINT _num, cv::Mat& image, std::wstring& image_base64)
{
    // JPG�t�H�[�}�b�g�ɃG���R�[�h
    std::vector<uchar> _JpgData;
    std::vector<int> compression_params = { cv::IMWRITE_JPEG_QUALITY, JPG_COMPRESS };  // 90%�̕i���ň��k
    cv::imencode(".jpg", image, _JpgData, compression_params);
    algorithm::encode_base64w(_JpgData, image_base64); // base64 �G���R�[�h
    //JPG�f�[�^�̃`�F�b�N�R�[�h
    if (CHK_JPG_BASE64)
    {
        saveImageDataToFile(_JpgData, "chk.jpg");
        saveImageAsHtmlw(image_base64, "chk.html");
    }
    SQLRETURN retCode = 0;

    SQLLEN BufferLength = (SQLLEN)image_base64.size()* sizeof(wchar_t); // �o�C�i���f�[�^�̃T�C�Y�� SQLLEN �^�Ŏw��
    //SQLSMALLINT DecimalDigits = 0;
    StrLen_or_IndPtr = BufferLength;
    //StrLen_or_IndPtr = SQL_NTS;
    retCode = SQLBindParameter(
        hStmt,
        _num,
        SQL_PARAM_INPUT,
        SQL_C_WCHAR,
        SQL_WLONGVARCHAR,
        //2147483647,
        //image_base64.size() * sizeof(wchar_t),
        BufferLength,
        0,
        (void*)image_base64.c_str(),
        BufferLength,
        (SQLLEN*)&StrLen_or_IndPtr
    );
#ifdef _DEBUG_SQL
    LOGMSG2("hStmt", hStmt);
    LOGMSG2("_num", _num);
    LOGMSG2("BufferLength", BufferLength);
    LOGMSG2("StrLen_or_IndPtr", StrLen_or_IndPtr);
    LOGMSG2("&StrLen_or_IndPtr", &StrLen_or_IndPtr);
    LOGMSG2("imageData.size()", image_base64.size());
    //saveImageAsHtml(image_base64, "chk.html");
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLBindParameter_Mat_b64w");
#endif
    return retCode;
}

#define Def_SQLBindParameter_Bin(hStmt, _num, imageData) \
{\
    SQLRETURN retCode = 0;\
    SQLLEN ColumnSize = 512*1024;\
    SQLLEN BufferLength = (SQLLEN)imageData.size();\
    SQLSMALLINT DecimalDigits = 0;\
    SQLLEN StrLen_or_IndPtr = NULL;\
    retCode = SQLBindParameter(\
        hStmt,\
        _num,\
        SQL_PARAM_INPUT,\
        SQL_C_BINARY,\
        SQL_LONGVARBINARY,\
        ColumnSize,\
        DecimalDigits,\
        imageData.data(),\
        BufferLength,\
        (SQLLEN*)&StrLen_or_IndPtr\
    );\
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"Def_SQLBindParameter_Bin");\
    return retCode;\
}


//////////////////////////////////////////////////////
//��肭���������[�`���BANSI��Unicode�����݂��Ă���̂ŕs������邩������Ȃ�
int Sql_ImageWrite_org(SqlServer& _sql_server, SqlImageOutput& _sql_image_data)
{
    SQLHENV hEnv = NULL;
    SQLHDBC hDbc = NULL;
    SQLHSTMT hStmt = NULL;
    SQLRETURN retCode;

    // ODBC�����m��
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);

    // �f�[�^�x�[�X�֐ڑ�
    std::wstring tempStr1;
    SQLWCHAR* connectionString = (SQLWCHAR*)make_connectionStringW(_sql_server, tempStr1);

    retCode = SQLDriverConnect(hDbc, NULL, connectionString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    //retCode = SQLDriverConnectW(hDbc, NULL, connectionString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"Connect to Server");

    // �X�e�[�g�����g�n���h�����m��
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    // SQL_ATTR_MAX_LENGTH ��ݒ肷��
    //SQLULEN maxLength = 10 * 1024 * 1024;
    SQLULEN maxLength = 2147483647;
    SQLSetStmtAttr(hStmt, SQL_ATTR_MAX_LENGTH, (SQLPOINTER)(uintptr_t)maxLength, SQL_IS_UINTEGER);

    // TEXTSIZE��ݒ�
    //const wchar_t* setTextSize = L"SET TEXTSIZE 10485760";
    const wchar_t* setTextSize = L"SET TEXTSIZE 2147483647";
    retCode = SQLExecDirect(hStmt, (SQLWCHAR*)setTextSize, SQL_NTS);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLExecDirect");
    //retCode = SQLPrepare(hStmt, (SQLWCHAR*)connectionString, SQL_NTS);
    //CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLPrepare");

    // �f�[�^
    std::wstring streamURL = _sql_image_data.StreamURL; //L"10.10.10.17";
    SQLBIGINT eventId = _sql_image_data.EventID; // �e�X�g�p�̃C�x���gID
    std::wstring Category = _sql_image_data.Category;
    std::wstring Location = _sql_image_data.Location;
    std::wstring timestampStr = _sql_image_data.Timestamp;
    int ImageWidth = _sql_image_data.ImageWidth;
    int ImageHeight = _sql_image_data.ImageHeight;
    //w�̂Ƃ���w��t����
    std::string image_base64;//image�̃e�L�X�g�������ݗp�̃o�b�t�@ SQLExecute�܂Ń������m�ۂ���K�v�����邽�߁B

    //SQLWCHAR* sqlQuery = (SQLWCHAR*)GenerateMergeSQL(L"dbo.image02");
    SQLWCHAR* sqlQuery = (SQLWCHAR*)GenerateMergeSQL(_sql_server.table.c_str());
    retCode = SQLPrepare(hStmt, sqlQuery, SQL_NTS);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"GenerateMergeSQL");
    if (!(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO))
        return 1; // Failed to prepare the statement
    
    // �p�����[�^�o�C���f�B���O
    SQLBindParameter_WString(hStmt, 1, streamURL);
    SQLBindParameter_Bigint(hStmt, 2, eventId);
    SQLBindParameter_WString(hStmt, 3, Category);
    SQLBindParameter_WString(hStmt, 4, Location);
    SQLBindParameter_WString(hStmt, 5, timestampStr);
    SQLBindParameter_Int(hStmt, 6, ImageWidth);
    SQLBindParameter_Int(hStmt, 7, ImageHeight);
 
    //SQLBindParameter_Mat_b64w(hStmt, 8, _sql_image_data.image, image_base64);
    SQLBindParameter_Mat_b64s(hStmt, 8, _sql_image_data.image, image_base64);

    // SQL �X�e�[�g�����g�̎��s
    retCode = SQLExecute(hStmt);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLExecute");

    // ���\�[�X�̉��
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

    return 0;
}

int Sql_ImageWriteS(SqlServer& _sql_server, SqlImageOutput& _sql_image_data)
{
    SQLHENV hEnv = NULL;
    SQLHDBC hDbc = NULL;
    SQLHSTMT hStmt = NULL;
    SQLRETURN retCode;

    // ODBC�����m��
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);

    // �f�[�^�x�[�X�֐ڑ� SQLDriverConnect��Unocode�����Ȃ�
    //std::wstring tempStr1W;
    //SQLWCHAR* connectionStringW = (SQLWCHAR*)make_connectionStringW(_sql_server, tempStr1W);//make_connectionStringS�͎g���Ȃ�
    //retCode = SQLDriverConnect(hDbc, NULL, connectionStringW, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    std::string tempStr1S;
    SQLWCHAR* connectionStringS = (SQLWCHAR*)make_connectionStringS(_sql_server, tempStr1S);//make_connectionStringS�͎g���Ȃ�
    retCode = SQLDriverConnect(hDbc, NULL, connectionStringS, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"Connect to Server");

    // �X�e�[�g�����g�n���h�����m��
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    // SQL_ATTR_MAX_LENGTH ��ݒ肷��
    //SQLULEN maxLength = 10 * 1024 * 1024;
    SQLULEN maxLength = 2147483647;
    SQLSetStmtAttr(hStmt, SQL_ATTR_MAX_LENGTH, (SQLPOINTER)(uintptr_t)maxLength, SQL_IS_UINTEGER);

    // TEXTSIZE��ݒ�
    const wchar_t* setTextSize = L"SET TEXTSIZE 2147483647";
    //const char* setTextSize = "SET TEXTSIZE 2147483647";
    retCode = SQLExecDirect(hStmt, (SQLWCHAR*)setTextSize, SQL_NTS);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLExecDirect");
    //retCode = SQLPrepare(hStmt, (SQLWCHAR*)connectionString, SQL_NTS);
    //CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLPrepare");

    // �f�[�^
    std::string streamURL = W2S(_sql_image_data.StreamURL); //L"10.10.10.17";
    SQLBIGINT eventId = _sql_image_data.EventID; // �e�X�g�p�̃C�x���gID
    std::string Category = W2S(_sql_image_data.Category);
    std::string Location = W2S(_sql_image_data.Location);
    std::string timestampStr = W2S(_sql_image_data.Timestamp);
    int ImageWidth = _sql_image_data.ImageWidth;
    int ImageHeight = _sql_image_data.ImageHeight;
    //w�̂Ƃ���w��t����
    std::string image_base64;//image�̃e�L�X�g�������ݗp�̃o�b�t�@ SQLExecute�܂Ń������m�ۂ���K�v�����邽�߁B

    //ASCII�ɏo���Ȃ�
    SQLWCHAR* sqlQuery = (SQLWCHAR*)GenerateMergeSQL(L"dbo.image03");
    //SQLCHAR* sqlQuery = (SQLCHAR*)GenerateMergeSQL(_sql_server.table.c_str());
    retCode = SQLPrepare(hStmt, sqlQuery, SQL_NTS);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"GenerateMergeSQL");
    if (!(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO))
        return 1; // Failed to prepare the statement

    // �p�����[�^�o�C���f�B���O
    SQLBindParameter_SString(hStmt, 1, streamURL);
    SQLBindParameter_Bigint(hStmt, 2, eventId);
    SQLBindParameter_SString(hStmt, 3, Category);
    SQLBindParameter_SString(hStmt, 4, Location);
    SQLBindParameter_SString(hStmt, 5, timestampStr);
    SQLBindParameter_Int(hStmt, 6, ImageWidth);
    SQLBindParameter_Int(hStmt, 7, ImageHeight);

    //SQLBindParameter_Mat_b64w(hStmt, 8, _sql_image_data.image, image_base64);
    SQLBindParameter_Mat_b64s(hStmt, 8, _sql_image_data.image, image_base64);

    // SQL �X�e�[�g�����g�̎��s
    retCode = SQLExecute(hStmt);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLExecute");

    // ���\�[�X�̉��
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

    return 0;
}

//////////////////////////////////////////////////////
//Unicode�œ��ꂷ��
int Sql_ImageWriteW(SqlServer& _sql_server, SqlImageOutput& _sql_image_data)
{
    SQLHENV hEnv = NULL;
    SQLHDBC hDbc = NULL;
    SQLHSTMT hStmt = NULL;
    SQLRETURN retCode;

    // ODBC�����m��
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);

    // �f�[�^�x�[�X�֐ڑ�
    std::wstring tempStr1;
    SQLWCHAR* connectionString = (SQLWCHAR*)make_connectionStringW(_sql_server, tempStr1);

    retCode = SQLDriverConnectW(hDbc, NULL, connectionString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"Connect to Server");

    // �X�e�[�g�����g�n���h�����m��
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    // SQL_ATTR_MAX_LENGTH ��ݒ肷��
    SQLULEN maxLength = 2147483647;
    SQLSetStmtAttrW(hStmt, SQL_ATTR_MAX_LENGTH, (SQLPOINTER)(uintptr_t)maxLength, SQL_IS_UINTEGER);

    // TEXTSIZE��ݒ�
    const wchar_t* setTextSize = L"SET TEXTSIZE 2147483647";
    retCode = SQLExecDirectW(hStmt, (SQLWCHAR*)setTextSize, SQL_NTS);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLExecDirect");
    //retCode = SQLPrepare(hStmt, (SQLWCHAR*)connectionString, SQL_NTS);
    //CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLPrepare");

    // �f�[�^
    std::wstring streamURL = _sql_image_data.StreamURL; //L"10.10.10.17";
    SQLBIGINT eventId = _sql_image_data.EventID; // �e�X�g�p�̃C�x���gID
    std::wstring Category = _sql_image_data.Category;
    std::wstring Location = _sql_image_data.Location;
    std::wstring timestampStr = _sql_image_data.Timestamp;
    int ImageWidth = _sql_image_data.ImageWidth;
    int ImageHeight = _sql_image_data.ImageHeight;
    std::wstring DataType = _sql_image_data.CompressType;

    //w�̂Ƃ���w��t����
    std::wstring image_base64;//image�̃e�L�X�g�������ݗp�̃o�b�t�@ SQLExecute�܂Ń������m�ۂ���K�v�����邽�߁B
    //std::string image_base64s;//image�̃e�L�X�g�������ݗp�̃o�b�t�@ SQLExecute�܂Ń������m�ۂ���K�v�����邽�߁B

    //SQLWCHAR* sqlQuery = (SQLWCHAR*)GenerateMergeSQL(L"dbo.image03");
    SQLWCHAR* sqlQuery = (SQLWCHAR*)GenerateMergeSQL(_sql_server.table.c_str());
    retCode = SQLPrepareW(hStmt, sqlQuery, SQL_NTS);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"GenerateMergeSQL");
    if (!(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO))
        return 1; // Failed to prepare the statement

    // �p�����[�^�o�C���f�B���O
    SQLBindParameter_WString(hStmt, 1, streamURL);
    SQLBindParameter_Bigint(hStmt, 2, eventId);
    SQLBindParameter_WString(hStmt, 3, Category);
    SQLBindParameter_WString(hStmt, 4, Location);
    SQLBindParameter_WString(hStmt, 5, timestampStr);
    SQLBindParameter_Int(hStmt, 6, ImageWidth);
    SQLBindParameter_Int(hStmt, 7, ImageHeight);

    SQLBindParameter_Mat_b64w(hStmt, 8, _sql_image_data.image, image_base64);
    //SQLBindParameter_Mat_b64s(hStmt, 8, _sql_image_data.image, image_base64s);
    // SQL �X�e�[�g�����g�̎��s
    retCode = SQLExecute(hStmt);
#ifdef _DEBUG_SQL
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLExecute");
#endif

    // ���\�[�X�̉��
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

    return 0;
}


int Sql_ImageWriteChunksW(SqlServer& _sql_server, SqlImageOutput& _sql_image_data)
{
    SQLHENV hEnv = NULL;
    SQLHDBC hDbc = NULL;
    SQLHSTMT hStmt = NULL;
    SQLRETURN retCode;

    // ODBC�����m��
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);

    // �f�[�^�x�[�X�֐ڑ�
    std::wstring tempStr1;
    SQLWCHAR* connectionString = (SQLWCHAR*)make_connectionStringW(_sql_server, tempStr1);

    retCode = SQLDriverConnectW(hDbc, NULL, connectionString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"Connect to Server");

    // �X�e�[�g�����g�n���h�����m��
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    SQLULEN maxLength = 2147483647;
    SQLSetStmtAttrW(hStmt, SQL_ATTR_MAX_LENGTH, (SQLPOINTER)(uintptr_t)maxLength, SQL_IS_UINTEGER);

    // TEXTSIZE��ݒ�
    const wchar_t* setTextSize = L"SET TEXTSIZE 2147483647";
    retCode = SQLExecDirectW(hStmt, (SQLWCHAR*)setTextSize, SQL_NTS);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLExecDirect");
    //retCode = SQLPrepare(hStmt, (SQLWCHAR*)connectionString, SQL_NTS);
    //CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLPrepare");

    // �f�[�^
    std::wstring streamURL = _sql_image_data.StreamURL; //L"10.10.10.17";
    SQLBIGINT eventId = _sql_image_data.EventID; // �e�X�g�p�̃C�x���gID
    std::wstring Category = _sql_image_data.Category;
    std::wstring Location = _sql_image_data.Location;
    std::wstring timestampStr = _sql_image_data.Timestamp;
    int ImageWidth = _sql_image_data.ImageWidth;
    int ImageHeight = _sql_image_data.ImageHeight;
    std::wstring DataType = _sql_image_data.CompressType;
    std::wstring image_base64;//image�̃e�L�X�g�������ݗp�̃o�b�t�@ SQLExecute�܂Ń������m�ۂ���K�v�����邽�߁B

    //�T�[�o�[�e�X�g
    //SQLWCHAR* sqlQuery = (SQLWCHAR*)GenerateMergeSQL(L"dbo.image03");

    SQLWCHAR* sqlQuery = (SQLWCHAR*)GenerateMergeSQL(_sql_server.table.c_str());
    retCode = SQLPrepareW(hStmt, sqlQuery, SQL_NTS);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"GenerateMergeSQL");
    if (!(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO))
        return 1; // Failed to prepare the statement

    // �p�����[�^�o�C���f�B���O
    SQLBindParameter_WString(hStmt, 1, streamURL);
    SQLBindParameter_Bigint(hStmt, 2, eventId);
    SQLBindParameter_WString(hStmt, 3, Category);
    SQLBindParameter_WString(hStmt, 4, Location);
    SQLBindParameter_WString(hStmt, 5, timestampStr);
    SQLBindParameter_Int(hStmt, 6, ImageWidth);
    SQLBindParameter_Int(hStmt, 7, ImageHeight);
    SQLBindParameter_WString(hStmt, 8, DataType);
    //SQLBindParameter_Mat_b64w(hStmt, 8, _sql_image_data.image, image_base64);

    // JPG�t�H�[�}�b�g�ɃG���R�[�h
    std::vector<uchar> _JpgData;
    //std::vector<int> compression_params = { cv::IMWRITE_JPEG_QUALITY, 90 };  // 90%�̕i���ň��k
    //cv::imencode(".jpg", _sql_image_data.image, _JpgData, compression_params);
    cv::imencode(".jpg", _sql_image_data.image, _JpgData, _sql_image_data.CompressOption);
    algorithm::encode_base64w(_JpgData, image_base64); // base64 �G���R�[�h

    //SQLLEN BufferLength = (SQLLEN)image_base64.size() * sizeof(wchar_t); // �o�C�i���f�[�^�̃T�C�Y�� SQLLEN �^�Ŏw��
    SQLLEN BufferLength = (SQLLEN)image_base64.size(); // �o�C�i���f�[�^�̃T�C�Y�� SQLLEN �^�Ŏw��

    // �p�����[�^�̃o�C���h�ݒ�
    //SQLLEN cbLargeText = SQL_LEN_DATA_AT_EXEC(0);
    //SQLLEN cbInt = 0, cbShortText = SQL_NTS;
    //SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WLONGVARCHAR, largeTextData.length(), 0, NULL, 0, &cbLargeText);

    SQLLEN lenOrInd = SQL_LEN_DATA_AT_EXEC(0);
    retCode = SQLBindParameter(
        hStmt,              // �X�e�[�g�����g�n���h��
        9,                  // �p�����[�^�ԍ�
        SQL_PARAM_INPUT,    // �p�����[�^�̕����A����
        SQL_C_WCHAR,        // C �f�[�^�^�C�v
        SQL_WLONGVARCHAR,   // SQL �f�[�^�^�C�v
        BufferLength,       // �J�����T�C�Y�i�����ł͕s�v�j
        0,                  // �����_�ȉ��̌���
        NULL,               // �p�����[�^�̃o�b�t�@�iNULL�Ŏ��s���ɒ񋟁j
        0,                  // �o�b�t�@���i�����ł�0�j
        &lenOrInd           // StrLen_or_IndPtr�i���s���f�[�^�̒����������j
    );

    // SQL �X�e�[�g�����g�̎��s
    int _put_count=0;
    retCode = SQLExecute(hStmt);
    if (retCode == SQL_NEED_DATA) 
    {
        SQLPOINTER token = NULL;
        while (SQLParamData(hStmt, &token) == SQL_NEED_DATA) 
        {
            // �e�L�X�g�f�[�^���`�����N�ɕ����đ��M
            size_t chunkSize = 8192;  // �`�����N�T�C�Y��1024����
            //size_t remaining = BufferLength;// image_base64.size();
            size_t remaining = image_base64.size();
            const wchar_t* pData = image_base64.c_str();
            while (remaining > 0) {
                size_t currentChunkSize = (remaining < chunkSize) ? remaining : chunkSize;
                SQLPutData(hStmt, (SQLPOINTER)pData, currentChunkSize * sizeof(wchar_t));
                pData += currentChunkSize;
                remaining -= currentChunkSize;
                _put_count++;
            }
        }
        if (CHK_JPG_BASE64)
        {
            saveImageDataToFile(_JpgData, "chk.jpg");
            saveImageAsHtmlw(image_base64, "chk.html");
        }
    }
#ifdef _DEBUG_SQL
    LOGMSG("SQLPutData()");
    LOGMSG2("ColumnSize", BufferLength);
    LOGMSG2("BufferLength", BufferLength);
    LOGMSG2("lenOrInd", lenOrInd);
    LOGMSG2("&lenOrInd", &lenOrInd);
    LOGMSG2("_sql_image_data.image.size()", _sql_image_data.image.size());
    LOGMSG2("_sql_image_data.image.total()* _sql_image_data.image.elemSize())", _sql_image_data.image.total()* _sql_image_data.image.elemSize());
    LOGMSG2("_JpgData.size()", _JpgData.size());
    LOGMSG2("image_base64.size()", image_base64.size());
    LOGMSG2("Sql_ImageWriteChunksW() _put_count", _put_count);
    LOGMSG2("Sql_ImageWriteChunksW() _put_count x 8192 = ", _put_count* 8192);
    CheckSQLError(retCode, hStmt, SQL_HANDLE_STMT, L"SQLExecute");
#endif

    // ���\�[�X�̉��
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

    return 0;
}
