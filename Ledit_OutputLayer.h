#ifndef LEDIT_OUTPUT_LAYER_H
#define LEDIT_OUTPUT_LAYER_H

typedef struct
{
    LLayer fixedLayer;
    int useSourceLayer;
    int skippedObjects;
} OutputLayerChoice;

static int OutputLayerIsWritable(LLayer layer)
{
    LLayerParamEx1512 params;
    return layer != NULL && !LLayer_IsDerived(layer) &&
           LLayer_GetParametersEx1512(layer, &params) == LStatusOK &&
           !params.Locked && !params.Protected;
}

/* The source-layer option is offered only for commands with a source per result. */
static int ChooseOutputLayer(LCell cell, int allowSourceLayer, OutputLayerChoice *choice)
{
    choice->fixedLayer = NULL;
    choice->useSourceLayer = 0;
    choice->skippedObjects = 0;
    if (cell == NULL)
        return 0;

    LFile file = LCell_GetFile(cell);
    LLayer current = LLayer_GetCurrent(file);
    LLayer layer;
    int firstFixedLayer = allowSourceLayer ? 1 : 0;
    int capacity = firstFixedLayer;
    for (layer = LLayer_GetList(file); layer != NULL; layer = LLayer_GetNext(layer))
        capacity++;

    if (capacity == firstFixedLayer)
    {
        LDialog_AlertBox("No writable output layer in the current design.");
        return 0;
    }

    LLayer *layers = malloc(capacity * sizeof(*layers));
    char (*names)[256] = malloc(capacity * sizeof(*names));
    const char **options = malloc(capacity * sizeof(*options));
    int count = firstFixedLayer;
    int defaultIndex = firstFixedLayer;
    int chosen = 0;
    if (layers != NULL && names != NULL && options != NULL)
    {
        if (allowSourceLayer)
            options[0] = "Use each object's source layer";
        for (layer = LLayer_GetList(file); layer != NULL; layer = LLayer_GetNext(layer))
        {
            if (!OutputLayerIsWritable(layer))
                continue;

            char name[192] = "";
            LLayer_GetName(layer, name, sizeof(name));
            name[sizeof(name) - 1] = '\0';
            const unsigned char *c = (const unsigned char *)name;
            while (*c >= 0x20 && *c <= 0x7e)
                c++;
            /* Fall back to the numeric layer and purpose for non-ASCII names. */
            const char *label = *c == '\0' && name[0] != '\0' ? name : "Layer";
            snprintf(names[count], sizeof(names[count]), "%s  [%d/%d]",
                     label, LLayer_GetLayerNumber(layer), LLayer_GetPurposeNumber(layer));
            options[count] = names[count];
            layers[count] = layer;
            if (layer == current)
                defaultIndex = count;
            count++;
        }

        if (count > firstFixedLayer)
        {
            int picked = LDialog_PickList("Output layer", options, count, defaultIndex);
            if (picked >= 0 && picked < count)
            {
                choice->useSourceLayer = picked < firstFixedLayer;
                if (!choice->useSourceLayer)
                    choice->fixedLayer = layers[picked];
                chosen = 1;
            }
        }
        else
            LDialog_AlertBox("No writable output layer in the current design.");
    }
    else
        LDialog_AlertBox("Could not load the output layer list.");

    free(options);
    free(names);
    free(layers);
    return chosen;
}

/* Commands that combine sources or draw new geometry need a single explicit layer. */
static LLayer PickOutputLayer(LCell cell)
{
    OutputLayerChoice choice;
    return ChooseOutputLayer(cell, 0, &choice) ? choice.fixedLayer : NULL;
}

static LLayer ResolveObjectOutputLayer(LCell cell, OutputLayerChoice *choice, LObject source)
{
    if (source == NULL)
        return NULL;
    if (!choice->useSourceLayer)
        return choice->fixedLayer;

    LLayer layer = LObject_GetLayer(cell, source);
    if (OutputLayerIsWritable(layer))
        return layer;
    choice->skippedObjects++;
    return NULL;
}

static void ReportSkippedOutputLayers(const OutputLayerChoice *choice)
{
    if (choice->skippedObjects != 0)
    {
        char message[128];
        snprintf(message, sizeof(message), "Skipped %d objects with unavailable or unwritable source layers.",
                 choice->skippedObjects);
        LDialog_AlertBox(message);
    }
}

#endif
