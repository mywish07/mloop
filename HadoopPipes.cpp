#include "HadoopPipes.h"
#include "hadoop/Pipes.hh"
#include <stdio.h>
#include "export.h"
#include "libhdfs/hdfs.h"
#include "pipes.hpp"
#include <cstring>
#include <string>
#include <string.h>
#include "LineReader.hpp"
#include <vector>

namespace hp = HadoopPipes;
using namespace hp;
using namespace std;

#define BUFF_SIZE 1000
extern "C" {

    CPointer mapContext_getInputValue(CPointer mapContext) {
        MapContext* context = (MapContext*) mapContext;
        std::string value = context->getInputValue();
        return (CPointer) value.c_str();
    }

    CPointer mapContext_getInputKey(CPointer mapContext) {
        MapContext* context = (MapContext*) mapContext;
        std::string value = context->getInputKey();
        return (CPointer) value.c_str();
    }

    void context_emit(CPointer taskContext, CPointer key, CPointer value) {
        TaskContext* context = (TaskContext*) taskContext;
        std::string k((char*) key), v((char*) value);
        context->emit(k, v);
    }

    void context_emitBatch(CPointer taskContextAddr, Objptr key, Objptr value, Int32 len) {
        hp::TaskContext* context = (hp::TaskContext*) taskContextAddr;
        char** kPtr = (char**) key;
        char** vPtr = (char**) value;
        for (int i = 0; i < len; i++) {
            std::string k((char*) kPtr[i]), v((char*) vPtr[i]);
            context->emit(k, v);
        }
    }

    CPointer reduceContext_getInputValue(CPointer reduceContext) {
        ReduceContext* context = (ReduceContext*) reduceContext;
        std::string value = context->getInputValue();
        return (CPointer) value.c_str();
    }

    int32_t reduceContext_nextValue(CPointer reduceContext) {
        ReduceContext* context = (ReduceContext*) reduceContext;
        return context->nextValue();
    }

    CPointer reduceContext_getInputKey(CPointer reduceContext) {
        ReduceContext* context = (ReduceContext*) reduceContext;
        std::string key = context->getInputKey();
        return (CPointer) key.c_str();
    }

    CPointer readInputSplitLine(CPointer reader_) {
        MloopRecordReader* reader = (MloopRecordReader*) reader_;
        char* newline = reader->getNewLine();
        LineReader* lineReader = reader->in;
        int bytesConsumed = lineReader->readLine(newline);
        reader->addStart(bytesConsumed);
        reader_updateOffset_bytesConsumed((Int64) reader->getStart(), (Int64) bytesConsumed);
        return (CPointer) newline;

    }

    void seekHdfs(CPointer reader_, Int64 pos) {
        MloopRecordReader* reader = (MloopRecordReader*) reader_;
        hdfsFile file = reader->getFile();
        hdfsFS fs = reader->getFs();
        hdfsSeek(fs, file, pos);
        reader->setStart(pos);
    }

    void setKeyValue(CPointer reader_, CPointer key, CPointer value) {
        MloopRecordReader* reader = (MloopRecordReader*) reader_;
        string* k = new string((char*) key);
        string* v = new string((char*) value);
        reader->setKeyValue(k, v);
    }

    void writer_write(CPointer writer_, CPointer key_, CPointer value_) {
        MloopRecordWriter* record_writer = (MloopRecordWriter*) writer_;
        string key((char*) key_);
        string value((char*) value_);
        LineWriter* writer = record_writer->getWriter();
        writer->write(key, value);
    }
}
