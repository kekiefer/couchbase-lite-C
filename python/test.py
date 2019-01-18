from CouchbaseLite.Database import Database, DatabaseConfiguration
from CouchbaseLite.Document import *

Database.deleteFile("db", "/tmp")

db = Database("db", DatabaseConfiguration("/tmp"));

print "db   = ", db
print "name = ", db.name
print "dir  = ", db.config
print "path = ", db.path
print "docs = ", db.count
print "seq  = ", db.lastSequence

assert(db.name == "db")
assert(db.path == "/tmp/db.cblite2")
assert(db.count == 0)
assert(db.lastSequence == 0)

with db:
    doc = db.getDocument("foo")
    assert(not doc)
    doc = db.getMutableDocument("foo")
    assert(not doc)
    doc = MutableDocument("foo")
    assert(doc.id == "foo")

    print "doc  = ", doc

    props = doc.properties
    print "props=", props
    assert(props == {})

    props["flavor"] = "cardamom"
    props["numbers"] = [1, 0, 3.125]
    doc["color"] = "green"
    print "props=", props

    db.save(doc)

    doc2 = db.getDocument("foo")
    assert(doc2.id == "foo")
    print "doc2 = ", doc2
    props2 = doc2.properties
    print "props2=", props2
    assert(props2 == props)
    assert(doc2["color"] == "green")

    assert(db.count == 1)
    assert(db.lastSequence == 1)

db.close()