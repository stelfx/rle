#include <stdint.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint32_t u32;

#define Assert(Expression) if(!(Expression)) {*(u32 *)0 = 0;}

typedef struct
{
    size_t FileSize;
    u8 *Contents;
} file_contents;

static file_contents
ReadEntireFileIntoMemory(char *FileName)
{
    file_contents Result = {0};

    FILE *File = fopen(FileName, "rb");
    if(File)
    {
        fseek(File,0,SEEK_END);
        Result.FileSize = ftell(File);
        fseek(File,0,SEEK_SET);

        Result.Contents = (u8 *)malloc(Result.FileSize);
        fread(Result.Contents, Result.FileSize, 1, File);

        fclose(File);

    }
    else
    {
        fprintf(stderr, "Unable to read file %s\n", FileName);
    }

    return Result;
}

static size_t
RLECompress(size_t InSize, u8 *In, size_t MaxOutSize, u8 *OutBase)
{
    u8 *Out = OutBase;
    
#define MAX_LITERAL_COUNT 255
#define MAX_RUN_COUNT 255
    
    u32 LiteralCount = 0;
    u8 Literals[MAX_LITERAL_COUNT];

    u8 *InEnd = In + InSize;
    
    while(In < InEnd)
    {
        u8 StartingValue = In[0];
        size_t Run = 1;
        while(Run < (size_t)(InEnd - In) &&
              (Run < MAX_RUN_COUNT) &&
              (In[Run] == StartingValue))
        {
            ++Run;
        }

        if((Run > 1) ||
           (LiteralCount == MAX_LITERAL_COUNT))
        {
            u8 LiteralCount8 = (u8)LiteralCount;
            Assert(LiteralCount8 == LiteralCount);
            *Out++ = LiteralCount8;
            
            for(u32 LiteralIndex = 0;
                LiteralIndex < LiteralCount;
                ++LiteralIndex)
            {
                *Out++ = Literals[LiteralIndex];
            }
            
            LiteralCount = 0;

            u8 Run8 = (u8)Run;
            Assert(Run8 == Run);
            *Out++ = Run8;
            *Out++ = StartingValue;

            In += Run;
        }
        else
        {
            Literals[LiteralCount++] = StartingValue;
            ++In;
        }
    }
#undef MAX_LITERAL_COUNT
#undef MAX_RUN_COUNT

    Assert(In == InEnd);

    size_t OutSize = Out - OutBase;
    Assert(OutSize <= MaxOutSize);

return OutSize;
}

static void
RLEDecompress(size_t InSize, u8 *In, size_t OutSize, u8 *Out)
{
    u8 *InEnd = In + InSize;
    while(In < InEnd)
    {
        u32 LiteralCount = *In++;
        while(LiteralCount--)
        {
            *Out++ = *In++;
        }

        u32 RepCount = *In++;
        u8 RepValue = *In++;
        while(RepCount--)
        {
            *Out++ = RepValue;
        }
    }
    
    Assert(In == InEnd);
}

int
main(int ArgCount, char **Args)
{    
    if(ArgCount == 4)
    {
        size_t FinalOutputSize = 0;
        u8 *FinalOutputBuffer = 0;
        
        char *Command = Args[1];
        char *InFileName = Args[2];
        char *OutFileName = Args[3];
        file_contents InFile = ReadEntireFileIntoMemory(InFileName);
        
        if(strcmp(Command, "compress") == 0)
        {
            size_t OutBufferSize = InFile.FileSize*2;
            u8 *OutBuffer = (u8 *)malloc(OutBufferSize + 4);
            size_t CompressedSize = RLECompress(InFile.FileSize, InFile.Contents, OutBufferSize, OutBuffer + 4);
            *(u32 *)OutBuffer = (u32)InFile.FileSize;

            FinalOutputSize = CompressedSize + 4;
            FinalOutputBuffer = OutBuffer;
        }
        else if(strcmp(Command, "decompress") == 0)
        {
            if(InFile.FileSize >= 4)
            {
                size_t OutBufferSize = *(u32 *)InFile.Contents;
                u8 *OutBuffer = (u8 *)malloc(OutBufferSize);
                RLEDecompress(InFile.FileSize - 4, InFile.Contents + 4, OutBufferSize, OutBuffer);

                FinalOutputSize = OutBufferSize;
                FinalOutputBuffer = OutBuffer;
            }
        }
        else
        {
            fprintf(stderr, "Usage: %s compress [raw filename] [compressed filename]\n", Args[0]);
            fprintf(stderr, "       %s decompress [compressed filename] [raw filename]\n", Args[0]);
        }
        
        if(FinalOutputBuffer)
        {
            FILE *OutFile = fopen(OutFileName, "wb");
            if(OutFile)
            {
                fwrite(FinalOutputBuffer, 1, FinalOutputSize, OutFile);
            }
            else
            {
                fprintf(stderr, "Unable to open output file %s\n", OutFileName);
            }
        }
    }
    else
    {
        fprintf(stderr, "Usage: %s compress [raw filename] [compressed filename]\n", Args[0]);
        fprintf(stderr, "       %s decompress [compressed filename] [raw filename]\n", Args[0]);
    }    
}
