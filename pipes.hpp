// BEGIN_COPYRIGHT
// 
// Copyright 2009-2014 CRS4.
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy
// of the License at
// 
//   http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.
// 
// END_COPYRIGHT

#ifndef HADOOP_PIPES_HPP
#define HADOOP_PIPES_HPP


#include <string>

#include "hadoop/SerialUtils.hh"
#include "hadoop/Pipes.hh"
#include <stdio.h>
#include "libhdfs/hdfs.h"
#include "LineReader.hpp"
#include "LineWriter.hpp"

namespace hu = HadoopUtils;
namespace hp = HadoopPipes;


class MloopMapper : public hp::Mapper {
private:
    char* addr;
public:

    MloopMapper(hp::MapContext& ctx);

    void map(hp::MapContext& ctx);

    void close();

    virtual ~MloopMapper();
};

class MloopReducer : public hp::Reducer {
public:

    MloopReducer() { 
    }
    MloopReducer(hp::ReduceContext& ctx);

    virtual void reduce(hp::ReduceContext& ctx);

    virtual void close();

    virtual ~MloopReducer();
};

class MloopCombiner : public MloopReducer {
public:
    MloopCombiner(hp::MapContext& ctx);
    virtual void reduce(hp::ReduceContext& ctx);
    virtual void close();
};

class MloopRecordReader : public hp::RecordReader {
private:
    hdfsFS fs;
    hdfsFile file;
    uint64_t offset;
    uint64_t start;
    uint64_t length;
    uint64_t bytes_read;
    std::string* key;
    std::string* value;
    char* newLine;
public:
    LineReader* in;
    MloopRecordReader(hp::MapContext& ctx);
    bool next(std::string& key, std::string& value);
    float getProgress();
    hdfsFS getFs();
    hdfsFile getFile();
    uint64_t getOffset();
    uint64_t getLength();
    uint64_t getBytes_read();
    void setBytes_read(uint64_t bytes);
    uint64_t getStart();
    void setStart(uint64_t start);
    void addStart(int bytesConsumed);
    void setKeyValue(std::string* key, std::string* value);
    char*& getNewLine();
    virtual void close();
    virtual ~MloopRecordReader();
};

class MloopRecordWriter : public hp::RecordWriter{
private:
    hdfsFS fs;
    hdfsFile file;
    LineWriter* writer;
public:
    MloopRecordWriter(){}
    MloopRecordWriter(hp::ReduceContext& ctx);
    LineWriter* getWriter();
    virtual void close();

    virtual void emit(const std::string& key, const std::string& value);

    virtual ~MloopRecordWriter();

};

class MloopFactory : public hp::Factory {
private:
    int32_t combine;
    int32_t reader;
    int32_t writer;
public:

    MloopFactory(int32_t combine, int32_t reader, int32_t writer) {
        this->combine = combine;
        this->reader = reader;
        this->writer = writer;
    }

    hp::Mapper* createMapper(hp::MapContext& ctx) const {
        return new MloopMapper(ctx);
    }

    hp::Reducer* createReducer(hp::ReduceContext& ctx) const {
        return new MloopReducer(ctx);
    }

    hp::RecordReader* createRecordReader(hp::MapContext& ctx) const {
        if (reader)
            return new MloopRecordReader(ctx);
        return NULL;
    }

    hp::Reducer* createCombiner(hp::MapContext& ctx) const {
        if (combine)
            return new MloopCombiner(ctx);
        return NULL;
    }

    hp::Partitioner* createPartitioner(hp::MapContext& ctx) const {
        return NULL;
    }

    hp::RecordWriter* createRecordWriter(hp::ReduceContext& ctx) const {
        printf("Trying to create writer...\n");
        if (writer)
            return new MloopRecordWriter(ctx);
        return NULL;
    }

};

#endif // HADOOP_PIPES_HPP
