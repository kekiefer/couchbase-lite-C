//
//  CBLDocument_Internal.hh
//
// Copyright (c) 2019 Couchbase, Inc All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once
#include "CBLDocument.h"
#include "Internal.hh"
#include "CBLDatabase_Internal.hh"
#include "c4.hh"
#include "c4Document+Fleece.h"
#include "fleece/Fleece.hh"
#include "fleece/Mutable.hh"
#include <unordered_map>

using namespace std;
using namespace fleece;

class CBLBlob;
class CBLNewBlob;


class CBLDocument : public CBLRefCounted {
public:
    // Construct a new document (not in any database yet)
    CBLDocument(const char *docID, bool isMutable);

    // Construct on an existing document
    CBLDocument(CBLDatabase *db, const string &docID, bool isMutable);

    // Mutable copy of another CBLDocument
    CBLDocument(const CBLDocument* otherDoc);

    // Document loaded from db without a C4Document (e.g. a replicator validation callback)
    CBLDocument(CBLDatabase *db,
                const string &docID,
                C4RevisionFlags revFlags,
                Dict body);

    static CBLDocument* containing(Value value) {
        C4Document* doc = c4doc_containingValue(value);
        return doc ? (CBLDocument*)doc->extraInfo.pointer : nullptr;
    }

    CBLDatabase* database() const               {return _db;}
    const char* docID() const                   {return _docID.c_str();}
    bool exists() const                         {return _c4doc != nullptr;}
    uint64_t sequence() const                   {return _c4doc ? _c4doc->sequence : 0;}
    bool isMutable() const                      {return _mutable;}

    FLDoc createFleeceDoc() const               {return c4doc_createFleeceDoc(_c4doc);}
    Dict properties() const;
    MutableDict mutableProperties()             {return properties().asMutable();}
    void setProperties(MutableDict d)           {if (checkMutable(nullptr)) _properties = d;}

    char* propertiesAsJSON() const;
    bool setPropertiesAsJSON(const char *json, C4Error* outError);

    RetainedConst<CBLDocument> save(CBLDatabase* db _cbl_nonnull,
                                    bool deleting,
                                    CBLConcurrencyControl concurrency,
                                    C4Error* outError);

    bool deleteDoc(CBLConcurrencyControl concurrency,
                   C4Error* outError);

    static bool deleteDoc(CBLDatabase* db _cbl_nonnull,
                          const char* docID _cbl_nonnull,
                          C4Error* outError);

    CBLBlob* getBlob(FLDict _cbl_nonnull);

    static void registerNewBlob(CBLNewBlob* _cbl_nonnull);
    static void unregisterNewBlob(CBLNewBlob* _cbl_nonnull);

private:
    CBLDocument(const string &docID, CBLDatabase *db, C4Document *d, bool isMutable);
    virtual ~CBLDocument();

    void initProperties();
    bool checkMutable(C4Error *outError) const;

    static string ensureDocID(const char *docID);

    static CBLNewBlob* findNewBlob(FLDict dict _cbl_nonnull);
    bool saveBlobs(CBLDatabase *db, C4Error *outError);

    using ValueToBlobMap = std::unordered_map<FLDict, Retained<CBLBlob>>;
    using UnretainedValueToBlobMap = std::unordered_map<FLDict, CBLNewBlob*>;

    static UnretainedValueToBlobMap* sNewBlobs;

    string const                _docID;                 // Document ID (never empty)
    Retained<CBLDatabase> const _db;                    // Database (null for new doc)
    c4::ref<C4Document> const   _c4doc;                 // LiteCore doc (null for new doc)
    RetainedValue               _properties;            // Properties, initialized lazily
    ValueToBlobMap              _blobs;
    bool const                  _mutable {false};       // True iff I am mutable
};
