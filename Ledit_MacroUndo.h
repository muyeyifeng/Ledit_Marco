#ifndef LEDIT_MACRO_UNDO_H
#define LEDIT_MACRO_UNDO_H

#include <stdint.h>
#include <limits.h>
#include <time.h>

#define MACRO_UNDO_JOURNAL "LeditMacroUndoJournalV1"
#define MACRO_UNDO_ARCHIVE "LeditUndoStep"
#define MACRO_UNDO_TARGET "LeditMacroUndoTargetV1"
#define MACRO_UNDO_BACKUP "LeditMacroUndoBackupV1"

typedef enum
{
    UndoCreatedObject,
    UndoChangedObject,
    UndoDeletedObject,
    UndoDeletedInstance,
    UndoCompletedEntry
} MacroUndoKind;

typedef struct
{
    uint64_t token;
    unsigned int id;
} MacroUndoTag;

typedef struct
{
    MacroUndoKind kind;
    unsigned int id;
    uint64_t layerFingerprint, fingerprint, backupFingerprint;
} MacroUndoEntry;

typedef struct
{
    uint64_t token;
    unsigned int count;
    char name[48], backup[40];
    char cell[MAX_CELL_NAME], view[MAX_CELL_NAME], library[MAX_CELL_NAME];
} MacroUndoHeader;

typedef struct
{
    LObject object;
    MacroUndoEntry entry;
    MacroUndoTag previousTag;
    int hadTag, tagged;
} PendingMacroUndoEntry;

typedef struct
{
    MacroUndoHeader header;
    LCell cell, backup;
    PendingMacroUndoEntry *entries;
    unsigned int count, capacity;
    unsigned int skipped;
} MacroUndoRun;

typedef struct
{
    LObject *objects;
    size_t count, capacity;
} MacroUndoInventory;

/* A macro invocation is synchronous. The published history is on the Cell,
   so modules compiled separately can read the same record. */
static MacroUndoRun *activeMacroUndo;

static int MacroUndoWritableLayer(LLayer layer)
{
    LLayerParamEx1512 params;
    return layer != NULL && !LLayer_IsDerived(layer) &&
           LLayer_GetParametersEx1512(layer, &params) == LStatusOK &&
           !params.Locked && !params.Protected;
}

static uint64_t HashUndoBytes(uint64_t hash, const void *bytes, size_t length)
{
    const unsigned char *p = bytes;
    while (length > 0)
    {
        hash = (hash ^ *p++) * UINT64_C(1099511628211);
        length--;
    }
    return hash;
}

static uint64_t HashUndoPoint(uint64_t hash, LPoint point)
{
    hash = HashUndoBytes(hash, &point.x, sizeof(point.x));
    return HashUndoBytes(hash, &point.y, sizeof(point.y));
}

static uint64_t FingerprintUndoLayer(LLayer layer)
{
    char name[MAX_LAYER_NAME] = "";
    LLayer_GetName(layer, name, sizeof(name));
    name[sizeof(name) - 1] = '\0';
    int number = LLayer_GetLayerNumber(layer);
    int purpose = LLayer_GetPurposeNumber(layer);
    uint64_t hash = HashUndoBytes(UINT64_C(14695981039346656037), name, strlen(name));
    hash = HashUndoBytes(hash, &number, sizeof(number));
    return HashUndoBytes(hash, &purpose, sizeof(purpose));
}

static int FingerprintUndoObject(LObject object, uint64_t *fingerprint)
{
    LShapeType shape = LObject_GetShape(object);
    LRect rect = LObject_GetMbb(object);
    uint64_t hash = UINT64_C(14695981039346656037);
    hash = HashUndoBytes(hash, &shape, sizeof(shape));
    hash = HashUndoBytes(hash, &rect.x0, sizeof(rect.x0));
    hash = HashUndoBytes(hash, &rect.y0, sizeof(rect.y0));
    hash = HashUndoBytes(hash, &rect.x1, sizeof(rect.x1));
    hash = HashUndoBytes(hash, &rect.y1, sizeof(rect.y1));
    if (shape == LCircle)
    {
        LCoord radius = LCircle_GetRadius(object);
        hash = HashUndoPoint(hash, LCircle_GetCenter(object));
        hash = HashUndoBytes(hash, &radius, sizeof(radius));
    }
    else if (shape == LPolygon || shape == LWire)
    {
        LVertex vertex;
        for (vertex = LObject_GetVertexList(object); vertex != NULL; vertex = LVertex_GetNext(vertex))
        {
            hash = HashUndoPoint(hash, LVertex_GetPoint(vertex));
            int curved = LVertex_HasCurve(vertex);
            hash = HashUndoBytes(hash, &curved, sizeof(curved));
            if (curved)
            {
                LPoint center;
                LArcDirection direction;
                if (LVertex_GetCurve(vertex, &center, &direction) != LStatusOK)
                    return 0;
                hash = HashUndoPoint(hash, center);
                hash = HashUndoBytes(hash, &direction, sizeof(direction));
            }
        }
        if (shape == LWire)
        {
            LCoord width = LWire_GetWidth(object);
            LJoinType join = LWire_GetJoinType(object);
            LCapType cap = LWire_GetCapType(object);
            short miter = LWire_GetMiterAngle(object);
            hash = HashUndoBytes(hash, &width, sizeof(width));
            hash = HashUndoBytes(hash, &join, sizeof(join));
            hash = HashUndoBytes(hash, &cap, sizeof(cap));
            hash = HashUndoBytes(hash, &miter, sizeof(miter));
        }
    }
    else if (shape == LTorus)
    {
        LTorusParams params;
        if (LTorus_GetParams(object, &params) != LStatusOK)
            return 0;
        hash = HashUndoPoint(hash, params.ptCenter);
        hash = HashUndoBytes(hash, &params.nInnerRadius, sizeof(params.nInnerRadius));
        hash = HashUndoBytes(hash, &params.nOuterRadius, sizeof(params.nOuterRadius));
        hash = HashUndoBytes(hash, &params.dStartAngle, sizeof(params.dStartAngle));
        hash = HashUndoBytes(hash, &params.dStopAngle, sizeof(params.dStopAngle));
    }
    else if (shape == LPie)
    {
        LPieParams params;
        if (LPie_GetParams(object, &params) != LStatusOK)
            return 0;
        hash = HashUndoPoint(hash, params.ptCenter);
        hash = HashUndoBytes(hash, &params.nRadius, sizeof(params.nRadius));
        hash = HashUndoBytes(hash, &params.dStartAngle, sizeof(params.dStartAngle));
        hash = HashUndoBytes(hash, &params.dStopAngle, sizeof(params.dStopAngle));
    }

    int dataType = LObject_GetGDSIIDataType(object);
    hash = HashUndoBytes(hash, &dataType, sizeof(dataType));
    uint64_t properties = 0;
    const char *name;
    for (name = LEntity_GetFirstProperty((LEntity)object); name != NULL; name = LEntity_GetNextProperty())
    {
        if (strcmp(name, MACRO_UNDO_TARGET) == 0 || strcmp(name, MACRO_UNDO_BACKUP) == 0)
            continue;
        uint64_t item = HashUndoBytes(UINT64_C(14695981039346656037), name, strlen(name));
        LPropertyType type;
        if (LEntity_GetPropertyType((LEntity)object, name, &type) != LStatusOK)
            return 0;
        item = HashUndoBytes(item, &type, sizeof(type));
        unsigned int size = LEntity_GetPropertyValueSize((LEntity)object, name);
        if (size != 0)
        {
            void *data = malloc(size);
            if (data == NULL)
                return 0;
            LStatus status = LEntity_GetPropertyValue((LEntity)object, name, data, size);
            if (status == LStatusOK)
                item = HashUndoBytes(item, data, size);
            free(data);
            if (status != LStatusOK && status != LPropertyHasNoValue)
                return 0;
        }
        properties += item;
    }
    *fingerprint = HashUndoBytes(hash, &properties, sizeof(properties));
    return 1;
}

static int ReadMacroUndoTag(LEntity entity, const char *property, MacroUndoTag *tag)
{
    return LEntity_PropertyExists(entity, property) == LStatusOK &&
           LEntity_GetPropertyValueSize(entity, property) == sizeof(*tag) &&
           LEntity_GetPropertyValue(entity, property, tag, sizeof(*tag)) == LStatusOK;
}

static LStatus WriteMacroUndoTag(LEntity entity, const char *property, uint64_t token, unsigned int id)
{
    MacroUndoTag tag = {0};
    tag.token = token;
    tag.id = id;
    return LEntity_AssignBlobProperty(entity, property, &tag, sizeof(tag));
}

/* The active journal stays in the original property. New records append a
   previous-step token after the entries. Legacy records end at the entries. */
static unsigned int MacroUndoJournalSize(unsigned int count)
{
    if (count > (UINT_MAX - sizeof(MacroUndoHeader) - sizeof(uint64_t)) / sizeof(MacroUndoEntry))
        return 0;
    return (unsigned int)(sizeof(MacroUndoHeader) + count * sizeof(MacroUndoEntry) + sizeof(uint64_t));
}

static void SetPreviousMacroUndoToken(MacroUndoHeader *journal, uint64_t token)
{
    unsigned int offset = MacroUndoJournalSize(journal->count) - sizeof(token);
    memcpy((unsigned char *)journal + offset, &token, sizeof(token));
}

static uint64_t PreviousMacroUndoToken(const MacroUndoHeader *journal)
{
    uint64_t token;
    unsigned int offset = MacroUndoJournalSize(journal->count) - sizeof(token);
    memcpy(&token, (const unsigned char *)journal + offset, sizeof(token));
    return token;
}

static int ReadMacroUndoJournal(LCell cell, const char *property, MacroUndoHeader **journal)
{
    *journal = NULL;
    LStatus status = LEntity_PropertyExists((LEntity)cell, property);
    if (status != LStatusOK)
        return status == LPropertyNotFound;

    unsigned int size = LEntity_GetPropertyValueSize((LEntity)cell, property);
    if (size < sizeof(MacroUndoHeader) || size > UINT_MAX - sizeof(uint64_t))
        return 0;
    MacroUndoHeader *header = malloc(size + sizeof(uint64_t));
    if (header == NULL)
        return 0;
    if (LEntity_GetPropertyValue((LEntity)cell, property, header, size) != LStatusOK)
    {
        free(header);
        return 0;
    }

    unsigned int expected = MacroUndoJournalSize(header->count);
    if (expected == 0 || (size != expected && size != expected - sizeof(uint64_t)) ||
        header->count == 0 || header->token == 0 ||
        header->name[sizeof(header->name) - 1] != '\0' || header->backup[sizeof(header->backup) - 1] != '\0' ||
        header->cell[sizeof(header->cell) - 1] != '\0' || header->view[sizeof(header->view) - 1] != '\0' ||
        header->library[sizeof(header->library) - 1] != '\0')
    {
        free(header);
        return 0;
    }
    if (size != expected)
        SetPreviousMacroUndoToken(header, 0);
    if (PreviousMacroUndoToken(header) >= header->token)
    {
        free(header);
        return 0;
    }
    *journal = header;
    return 1;
}

static void MacroUndoArchiveName(char *name, size_t size, uint64_t token)
{
    snprintf(name, size, MACRO_UNDO_ARCHIVE "%08x%08x",
             (unsigned int)(token >> 32), (unsigned int)token);
}

static void ReadMacroUndoOwner(LCell cell, MacroUndoHeader *header)
{
    LCell_GetName(cell, header->cell, sizeof(header->cell));
    LCell_GetViewName(cell, header->view, sizeof(header->view));
    LCell_GetLibName(cell, header->library, sizeof(header->library));
    header->cell[sizeof(header->cell) - 1] = '\0';
    header->view[sizeof(header->view) - 1] = '\0';
    header->library[sizeof(header->library) - 1] = '\0';
}

static int IsMacroUndoOwner(LCell cell, const MacroUndoHeader *header)
{
    MacroUndoHeader owner = {0};
    ReadMacroUndoOwner(cell, &owner);
    return strcmp(owner.cell, header->cell) == 0 && strcmp(owner.view, header->view) == 0 &&
           strcmp(owner.library, header->library) == 0;
}

static LCell FindMacroUndoBackup(LCell cell, const MacroUndoHeader *header)
{
    if (header->backup[0] == '\0')
        return NULL;
    LCell backup = LCell_Find(LCell_GetFile(cell), header->backup);
    MacroUndoTag tag;
    if (backup == NULL || backup == cell ||
        !ReadMacroUndoTag((LEntity)backup, MACRO_UNDO_BACKUP, &tag) ||
        tag.token != header->token || tag.id != 0)
        return NULL;
    return backup;
}

static LCell CreateMacroUndoBackup(MacroUndoRun *run)
{
    if (run->backup != NULL)
        return run->backup;
    LFile file = LCell_GetFile(run->cell);
    unsigned int suffix;
    for (suffix = 1; suffix <= 10000; suffix++)
    {
        snprintf(run->header.backup, sizeof(run->header.backup), "LeditUndo_%u_%u",
                 (unsigned int)run->header.token, suffix);
        if (LCell_Find(file, run->header.backup) != NULL)
            continue;
        LCell backup = LCell_New(file, run->header.backup);
        if (backup == NULL)
            break;
        if ((LCell_GetVisible() != run->cell && LCell_MakeVisibleNoRefresh(run->cell) != LStatusOK) ||
            WriteMacroUndoTag((LEntity)backup, MACRO_UNDO_BACKUP, run->header.token, 0) != LStatusOK ||
            LCell_SetShowInLists(backup, LFALSE) != LStatusOK)
        {
            LCell_Delete(backup);
            break;
        }
        run->backup = backup;
        return backup;
    }
    run->header.backup[0] = '\0';
    return NULL;
}

static int AppendPendingMacroUndo(MacroUndoRun *run, LObject object, MacroUndoEntry entry)
{
    if (run->count == run->capacity)
    {
        unsigned int capacity = run->capacity ? run->capacity * 2 : 32;
        if (capacity <= run->capacity || MacroUndoJournalSize(capacity) == 0 ||
            (size_t)capacity > (size_t)-1 / sizeof(*run->entries))
            return 0;
        PendingMacroUndoEntry *entries = realloc(run->entries, (size_t)capacity * sizeof(*entries));
        if (entries == NULL)
            return 0;
        run->entries = entries;
        run->capacity = capacity;
    }
    memset(&run->entries[run->count], 0, sizeof(run->entries[run->count]));
    run->entries[run->count].object = object;
    run->entries[run->count].entry = entry;
    run->count++;
    return 1;
}

/* Called before in-place edits and deletes. An object whose snapshot fails
   is left untouched so it never becomes an unrecorded destructive edit. */
static PendingMacroUndoEntry *BackUpMacroObject(LCell cell, LObject object, MacroUndoKind kind)
{
    MacroUndoRun *run = activeMacroUndo;
    if (run == NULL || cell != run->cell || object == NULL)
        return NULL;
    unsigned int i;
    for (i = 0; i < run->count; i++)
    {
        if (run->entries[i].object == object)
            return &run->entries[i];
    }

    MacroUndoEntry entry = {0};
    entry.kind = kind;
    entry.id = run->count + 1;
    LLayer layer = kind == UndoDeletedInstance ? NULL : LObject_GetLayer(cell, object);
    if (kind != UndoDeletedInstance && !MacroUndoWritableLayer(layer))
        return NULL;
    if (kind == UndoChangedObject && !FingerprintUndoObject(object, &entry.fingerprint))
        return NULL;
    LCell backup = CreateMacroUndoBackup(run);
    if (backup == NULL)
        return NULL;

    if (kind == UndoDeletedInstance)
    {
        LInstance original = LObject_GetInstance(object);
        if (original == NULL || LInstance_GetLock(original))
            return NULL;
        LInstance copy = LInstance_New_Ex99(backup, LInstance_GetCell(original),
                                          LInstance_GetTransform_Ex99(original),
                                          LInstance_GetRepeatCount(original), LInstance_GetDelta(original));
        if (copy == NULL)
            return NULL;

        char name[256] = "";
        LInstance_GetName(original, name, sizeof(name));
        name[sizeof(name) - 1] = '\0';
        if ((*name && LInstance_SetName(backup, copy, name) != LStatusOK) ||
            LEntity_CopyAllProperties((LEntity)copy, (LEntity)original) != LStatusOK ||
            WriteMacroUndoTag((LEntity)copy, MACRO_UNDO_BACKUP, run->header.token, entry.id) != LStatusOK ||
            !AppendPendingMacroUndo(run, object, entry))
        {
            LInstance_Delete(backup, copy);
            return NULL;
        }
    }
    else
    {
        entry.layerFingerprint = FingerprintUndoLayer(layer);
        LObject copy = LObject_Copy(backup, layer, object);
        if (copy == NULL || copy == object)
            return NULL;
        if (LEntity_CopyAllProperties((LEntity)copy, (LEntity)object) != LStatusOK ||
            !FingerprintUndoObject(copy, &entry.backupFingerprint) ||
            WriteMacroUndoTag((LEntity)copy, MACRO_UNDO_BACKUP, run->header.token, entry.id) != LStatusOK ||
            !AppendPendingMacroUndo(run, object, entry))
        {
            LObject_Delete(backup, copy);
            return NULL;
        }
    }
    return &run->entries[run->count - 1];
}

static int RecordMacroObjectEdit(LCell cell, LObject object)
{
    if (activeMacroUndo == NULL)
        return 1;
    if (BackUpMacroObject(cell, object, UndoChangedObject) != NULL)
        return 1;
    activeMacroUndo->skipped++;
    return 0;
}

static LStatus DeleteMacroObject(LCell cell, LObject object)
{
    if (activeMacroUndo == NULL)
        return LObject_Delete(cell, object);
    MacroUndoKind kind = LObject_GetShape(object) == LObjInstance ? UndoDeletedInstance : UndoDeletedObject;
    PendingMacroUndoEntry *pending = BackUpMacroObject(cell, object, kind);
    if (pending == NULL)
    {
        activeMacroUndo->skipped++;
        return LBadParameters;
    }
    if (pending->entry.kind == UndoCompletedEntry)
        return LBadParameters;
    LStatus status = kind == UndoDeletedInstance ?
                     LInstance_Delete(cell, LObject_GetInstance(object)) : LObject_Delete(cell, object);
    if (status == LStatusOK)
    {
        pending->entry.kind = kind;
        pending->object = NULL;
    }
    else if (pending->entry.kind != UndoChangedObject)
        pending->entry.kind = UndoCompletedEntry;
    return status;
}

static int CompareMacroUndoHandles(const void *a, const void *b)
{
    uintptr_t first = (uintptr_t)*(const LObject *)a;
    uintptr_t second = (uintptr_t)*(const LObject *)b;
    return (first > second) - (first < second);
}

static int CaptureMacroUndoInventory(LCell cell, MacroUndoInventory *inventory)
{
    LFile file = LCell_GetFile(cell);
    LLayer layer;
    for (layer = LLayer_GetList(file); layer != NULL; layer = LLayer_GetNext(layer))
    {
        LObject object;
        for (object = LObject_GetList(cell, layer); object != NULL; object = LObject_GetNext(object))
        {
            if (inventory->count == inventory->capacity)
            {
                size_t capacity = inventory->capacity ? inventory->capacity * 2 : 128;
                if (capacity <= inventory->capacity || capacity > (size_t)-1 / sizeof(*inventory->objects))
                    return 0;
                LObject *objects = realloc(inventory->objects, capacity * sizeof(*objects));
                if (objects == NULL)
                    return 0;
                inventory->objects = objects;
                inventory->capacity = capacity;
            }
            inventory->objects[inventory->count++] = object;
        }
    }
    if (inventory->count > 1)
        qsort(inventory->objects, inventory->count, sizeof(*inventory->objects), CompareMacroUndoHandles);
    return 1;
}

static int MacroUndoContainsObject(const MacroUndoInventory *inventory, LObject object)
{
    return inventory->count != 0 && bsearch(&object, inventory->objects, inventory->count,
                                           sizeof(*inventory->objects), CompareMacroUndoHandles) != NULL;
}

static LStatus SaveMacroUndoJournal(LCell cell, const char *property, const MacroUndoHeader *journal)
{
    return LEntity_AssignBlobProperty((LEntity)cell, property, journal,
                                     MacroUndoJournalSize(journal->count));
}

static void DiscardMacroUndoBackup(LCell cell, const MacroUndoHeader *journal)
{
    LCell backup = FindMacroUndoBackup(cell, journal);
    if (backup != NULL)
        LCell_Delete(backup);
}

static int CollectMacroUndoChanges(MacroUndoRun *run, const MacroUndoInventory *before,
                                    const MacroUndoInventory *after)
{
    unsigned int i;
    for (i = 0; i < run->count; i++)
    {
        PendingMacroUndoEntry *pending = &run->entries[i];
        if (pending->entry.kind != UndoChangedObject)
            continue;
        uint64_t fingerprint;
        if (!MacroUndoContainsObject(after, pending->object) ||
            !FingerprintUndoObject(pending->object, &fingerprint))
            return 0;
        if (fingerprint == pending->entry.fingerprint)
            pending->entry.kind = UndoCompletedEntry;
        else
            pending->entry.fingerprint = fingerprint;
    }
    size_t index;
    for (index = 0; index < after->count; index++)
    {
        LObject object = after->objects[index];
        if ((index > 0 && object == after->objects[index - 1]) || MacroUndoContainsObject(before, object))
            continue;
        LLayer layer = LObject_GetLayer(run->cell, object);
        MacroUndoEntry entry = {0};
        entry.kind = UndoCreatedObject;
        entry.id = run->count + 1;
        if (layer == NULL || !FingerprintUndoObject(object, &entry.fingerprint))
            return 0;
        entry.layerFingerprint = FingerprintUndoLayer(layer);
        if (!AppendPendingMacroUndo(run, object, entry))
            return 0;
    }
    return 1;
}

static int PublishMacroUndo(MacroUndoRun *run, const MacroUndoHeader *previous)
{
    unsigned int size = MacroUndoJournalSize(run->count);
    MacroUndoHeader *journal = size == 0 ? NULL : malloc(size);
    if (journal == NULL)
        return 0;
    run->header.count = run->count;
    *journal = run->header;
    SetPreviousMacroUndoToken(journal, previous != NULL ? previous->token : 0);
    MacroUndoEntry *entries = (MacroUndoEntry *)(journal + 1);
    unsigned int i;
    int published = 0;
    for (i = 0; i < run->count; i++)
    {
        PendingMacroUndoEntry *pending = &run->entries[i];
        entries[i] = pending->entry;
        if (entries[i].kind != UndoChangedObject && entries[i].kind != UndoCreatedObject)
            continue;
        pending->hadTag = ReadMacroUndoTag((LEntity)pending->object, MACRO_UNDO_TARGET, &pending->previousTag);
        if (WriteMacroUndoTag((LEntity)pending->object, MACRO_UNDO_TARGET,
                              journal->token, entries[i].id) != LStatusOK)
            break;
        pending->tagged = 1;
    }
    if (i == run->count)
    {
        /* Archive the old head before replacing it, leaving the current
           journal readable if either property write fails. */
        char archive[sizeof(MACRO_UNDO_ARCHIVE) + 16];
        int archived = 0;
        if (previous != NULL)
        {
            MacroUndoArchiveName(archive, sizeof(archive), previous->token);
            archived = SaveMacroUndoJournal(run->cell, archive, previous) == LStatusOK;
        }
        if (previous == NULL || archived)
            published = SaveMacroUndoJournal(run->cell, MACRO_UNDO_JOURNAL, journal) == LStatusOK;
        if (archived && !published)
            LEntity_DeleteProperty((LEntity)run->cell, archive);
    }
    if (!published)
    {
        for (i = 0; i < run->count; i++)
        {
            PendingMacroUndoEntry *pending = &run->entries[i];
            if (!pending->tagged)
                continue;
            if (pending->hadTag)
                WriteMacroUndoTag((LEntity)pending->object, MACRO_UNDO_TARGET,
                                  pending->previousTag.token, pending->previousTag.id);
            else
                LEntity_DeleteProperty((LEntity)pending->object, MACRO_UNDO_TARGET);
        }
    }
    free(journal);
    return published;
}

static void RunWithMacroUndo(const char *name, void (*command)(void))
{
    LCell cell = LCell_GetVisible();
    if (cell == NULL)
    {
        LDialog_AlertBox("Open a cell before running this macro.");
        return;
    }

    MacroUndoTag backupTag;
    if (ReadMacroUndoTag((LEntity)cell, MACRO_UNDO_BACKUP, &backupTag) && backupTag.id == 0)
    {
        LDialog_AlertBox("Open the original cell to edit it with macro undo.");
        return;
    }

    MacroUndoHeader *previous;
    MacroUndoInventory before = {0}, after = {0};
    if (!ReadMacroUndoJournal(cell, MACRO_UNDO_JOURNAL, &previous) ||
        (previous != NULL && previous->token == UINT64_MAX) ||
        !CaptureMacroUndoInventory(cell, &before))
    {
        free(previous);
        free(before.objects);
        LDialog_AlertBox("Could not prepare macro undo. No changes were made.");
        return;
    }

    MacroUndoRun run = {0};
    run.cell = cell;
    run.header.token = previous != NULL ? previous->token + 1 : ((uint64_t)time(NULL) << 16) + 1;
    if (previous != NULL && !IsMacroUndoOwner(cell, previous))
    {
        free(previous);
        previous = NULL;
    }
    ReadMacroUndoOwner(cell, &run.header);
    snprintf(run.header.name, sizeof(run.header.name), "%s", name);
    activeMacroUndo = &run;
    command();
    activeMacroUndo = NULL;

    int recorded = LCell_GetVisible() == cell && CaptureMacroUndoInventory(cell, &after) &&
                   CollectMacroUndoChanges(&run, &before, &after);
    unsigned int changes = 0, i;
    for (i = 0; i < run.count; i++)
        changes += run.entries[i].entry.kind != UndoCompletedEntry;
    if (recorded && changes != 0)
        recorded = PublishMacroUndo(&run, previous);
    else if (recorded)
        DiscardMacroUndoBackup(cell, &run.header);

    if (!recorded)
    {
        char message[160];
        snprintf(message, sizeof(message), "Could not save the macro undo record.%s%s",
                 run.backup != NULL ? " Original objects remain in backup cell " : "",
                 run.backup != NULL ? run.header.backup : "");
        LDialog_AlertBox(message);
    }
    if (run.skipped > 0)
    {
        char message[150];
        snprintf(message, sizeof(message), "Skipped %u objects that could not be backed up for undo.", run.skipped);
        LDialog_AlertBox(message);
    }
    free(run.entries);
    free(after.objects);
    free(before.objects);
    free(previous);
}

typedef struct
{
    LLayer layer;
    LObject target, backup, restored;
    LInstance backupInstance, restoredInstance;
} ResolvedMacroUndoEntry;

static int AssignUndoHandles(const MacroUndoInventory *objects, const MacroUndoHeader *journal,
                              const MacroUndoEntry *entries, ResolvedMacroUndoEntry *resolved, int backup)
{
    size_t i;
    for (i = 0; i < objects->count; i++)
    {
        if (i > 0 && objects->objects[i] == objects->objects[i - 1])
            continue;
        LObject object = objects->objects[i];
        MacroUndoTag tag;
        if (!ReadMacroUndoTag((LEntity)object, backup ? MACRO_UNDO_BACKUP : MACRO_UNDO_TARGET, &tag) ||
            tag.token != journal->token || tag.id == 0 || tag.id > journal->count)
            continue;
        unsigned int index = tag.id - 1;
        MacroUndoKind kind = entries[index].kind;
        if (backup ? (kind != UndoChangedObject && kind != UndoDeletedObject) :
                     (kind != UndoChangedObject && kind != UndoCreatedObject))
            continue;
        LObject *handle = backup ? &resolved[index].backup : &resolved[index].target;
        if (*handle != NULL)
            return 0;
        *handle = object;
    }
    return 1;
}

static int ResolveLastMacroUndo(LCell cell, LCell backup, const MacroUndoHeader *journal,
                                const MacroUndoEntry *entries, ResolvedMacroUndoEntry *resolved)
{
    MacroUndoInventory live = {0}, saved = {0};
    int valid = CaptureMacroUndoInventory(cell, &live) &&
                AssignUndoHandles(&live, journal, entries, resolved, 0);
    if (valid && backup != NULL)
    {
        valid = CaptureMacroUndoInventory(backup, &saved) &&
                AssignUndoHandles(&saved, journal, entries, resolved, 1);
        LInstance instance;
        for (instance = LInstance_GetList(backup); valid && instance != NULL; instance = LInstance_GetNext(instance))
        {
            MacroUndoTag tag;
            if (!ReadMacroUndoTag((LEntity)instance, MACRO_UNDO_BACKUP, &tag) ||
                tag.token != journal->token || tag.id == 0 || tag.id > journal->count ||
                entries[tag.id - 1].kind != UndoDeletedInstance)
                continue;
            ResolvedMacroUndoEntry *entry = &resolved[tag.id - 1];
            valid = entry->backupInstance == NULL;
            entry->backupInstance = instance;
        }
    }
    free(saved.objects);
    free(live.objects);

    unsigned int i;
    for (i = 0; valid && i < journal->count; i++)
    {
        const MacroUndoEntry *entry = &entries[i];
        ResolvedMacroUndoEntry *item = &resolved[i];
        if (entry->id != i + 1)
            return 0;
        if (entry->kind == UndoCompletedEntry)
            continue;
        if (entry->kind == UndoDeletedInstance)
        {
            valid = item->backupInstance != NULL;
            continue;
        }
        if (entry->kind != UndoCreatedObject && entry->kind != UndoChangedObject && entry->kind != UndoDeletedObject)
            return 0;

        LLayer targetLayer = item->target != NULL ? LObject_GetLayer(cell, item->target) : NULL;
        LLayer backupLayer = item->backup != NULL ? LObject_GetLayer(backup, item->backup) : NULL;
        item->layer = targetLayer != NULL ? targetLayer : backupLayer;
        valid = MacroUndoWritableLayer(item->layer) &&
                FingerprintUndoLayer(item->layer) == entry->layerFingerprint;
        if (entry->kind == UndoChangedObject || entry->kind == UndoCreatedObject)
        {
            uint64_t fingerprint;
            valid = valid && targetLayer == item->layer &&
                    FingerprintUndoObject(item->target, &fingerprint) && fingerprint == entry->fingerprint;
        }
        if (entry->kind == UndoChangedObject || entry->kind == UndoDeletedObject)
        {
            uint64_t fingerprint;
            valid = valid && backupLayer == item->layer &&
                    FingerprintUndoObject(item->backup, &fingerprint) && fingerprint == entry->backupFingerprint;
        }
    }
    return valid;
}

static void RemoveStagedMacroUndo(LCell cell, ResolvedMacroUndoEntry *entry)
{
    if (entry->restored != NULL)
        LObject_Delete(cell, entry->restored);
    if (entry->restoredInstance != NULL)
        LInstance_Delete(cell, entry->restoredInstance);
}

static int ClearUndoBackupTag(LEntity object)
{
    /* Keep the target tag copied from the original so the previous step can
       still identify this object after it has been restored. */
    return LEntity_PropertyExists(object, MACRO_UNDO_BACKUP) == LPropertyNotFound ||
           LEntity_DeleteProperty(object, MACRO_UNDO_BACKUP) == LStatusOK;
}

static int StageMacroUndoRestore(LCell cell, const MacroUndoEntry *entries, unsigned int count,
                                  ResolvedMacroUndoEntry *resolved)
{
    unsigned int i;
    for (i = 0; i < count; i++)
    {
        ResolvedMacroUndoEntry *item = &resolved[i];
        if (entries[i].kind == UndoChangedObject || entries[i].kind == UndoDeletedObject)
        {
            item->restored = LObject_Copy(cell, item->layer, item->backup);
            if (item->restored == item->backup)
                item->restored = NULL;
            if (item->restored == NULL ||
                LEntity_CopyAllProperties((LEntity)item->restored, (LEntity)item->backup) != LStatusOK ||
                !ClearUndoBackupTag((LEntity)item->restored))
                break;
        }
        else if (entries[i].kind == UndoDeletedInstance)
        {
            LInstance backup = item->backupInstance;
            item->restoredInstance = LInstance_New_Ex99(cell, LInstance_GetCell(backup),
                                                        LInstance_GetTransform_Ex99(backup),
                                                        LInstance_GetRepeatCount(backup),
                                                        LInstance_GetDelta(backup));
            if (item->restoredInstance == NULL)
                break;
            char name[256] = "";
            LInstance_GetName(backup, name, sizeof(name));
            name[sizeof(name) - 1] = '\0';
            if ((*name && LInstance_SetName(cell, item->restoredInstance, name) != LStatusOK) ||
                LEntity_CopyAllProperties((LEntity)item->restoredInstance, (LEntity)backup) != LStatusOK ||
                !ClearUndoBackupTag((LEntity)item->restoredInstance))
                break;
        }
    }
    if (i == count)
        return 1;
    unsigned int staged;
    for (staged = 0; staged <= i; staged++)
        RemoveStagedMacroUndo(cell, &resolved[staged]);
    return 0;
}

/* Validate all targets and stage all replacement objects before removing
   anything from the working cell. Unrelated geometry is never rewritten. */
static void UndoLastMacroInCell(void)
{
    LCell cell = LCell_GetVisible();
    MacroUndoHeader *journal = NULL;
    if (cell != NULL && !ReadMacroUndoJournal(cell, MACRO_UNDO_JOURNAL, &journal))
    {
        LDialog_AlertBox("Could not read the macro undo history for the current cell.");
        return;
    }
    if (journal == NULL || !IsMacroUndoOwner(cell, journal))
    {
        free(journal);
        LDialog_AlertBox("No macro undo record for the current cell.");
        return;
    }

    MacroUndoHeader *previous = NULL;
    char archive[sizeof(MACRO_UNDO_ARCHIVE) + 16];
    uint64_t previousToken = PreviousMacroUndoToken(journal);
    if (previousToken != 0)
    {
        MacroUndoArchiveName(archive, sizeof(archive), previousToken);
        if (!ReadMacroUndoJournal(cell, archive, &previous) || previous == NULL ||
            previous->token != previousToken || !IsMacroUndoOwner(cell, previous))
        {
            free(previous);
            free(journal);
            LDialog_AlertBox("Could not read the previous macro undo record. No changes were made.");
            return;
        }
    }

    LCell backup = FindMacroUndoBackup(cell, journal);
    MacroUndoEntry *entries = (MacroUndoEntry *)(journal + 1);
    ResolvedMacroUndoEntry *resolved = calloc(journal->count, sizeof(*resolved));
    if (resolved == NULL || !ResolveLastMacroUndo(cell, backup, journal, entries, resolved))
    {
        free(resolved);
        free(previous);
        free(journal);
        LDialog_AlertBox("Cannot undo this macro. Its objects, layers, or backup have changed.");
        return;
    }
    if (!StageMacroUndoRestore(cell, entries, journal->count, resolved))
    {
        free(resolved);
        free(previous);
        free(journal);
        LDisplay_Refresh();
        LDialog_AlertBox("Could not restore the original geometry. The macro undo record was kept.");
        return;
    }

    unsigned int i, failed = 0;
    for (i = 0; i < journal->count; i++)
    {
        if ((entries[i].kind == UndoCreatedObject || entries[i].kind == UndoChangedObject) &&
            LObject_Delete(cell, resolved[i].target) != LStatusOK)
        {
            RemoveStagedMacroUndo(cell, &resolved[i]);
            failed++;
        }
        else
            entries[i].kind = UndoCompletedEntry;
    }
    free(resolved);
    LDisplay_Refresh();

    LStatus popped = LPropertyNotFound;
    if (failed == 0)
    {
        if (previous != NULL)
            popped = SaveMacroUndoJournal(cell, MACRO_UNDO_JOURNAL, previous);
        else
            popped = LEntity_DeleteProperty((LEntity)cell, MACRO_UNDO_JOURNAL);
    }

    if (popped == LStatusOK)
    {
        if (previous != NULL)
            LEntity_DeleteProperty((LEntity)cell, archive);
        DiscardMacroUndoBackup(cell, journal);
        char message[100];
        snprintf(message, sizeof(message), "Undid macro: %s", journal->name);
        LStatusBar_SetMsg(message);
    }
    else if (SaveMacroUndoJournal(cell, MACRO_UNDO_JOURNAL, journal) != LStatusOK)
        LDialog_AlertBox("Could not update the macro undo record. The backup was kept.");
    else if (failed != 0)
        LDialog_AlertBox("Macro undo was partial. Run Undo Last Macro to retry the remaining objects.");
    else
        LDialog_AlertBox("The objects were restored, but undo history could not advance. Run Undo Last Macro to retry.");
    free(previous);
    free(journal);
}

#define DEFINE_UNDOABLE_MACRO(wrapper, command, label) \
    void wrapper(void) { RunWithMacroUndo(label, command); }

#endif
