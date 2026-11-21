#ifndef LEDIT_OUTPUT_LAYER_H
#define LEDIT_OUTPUT_LAYER_H

/* Pick once per command. NULL means cancelled or no writable layer. */
static LLayer PickOutputLayer(LCell cell)
{
    if (cell == NULL)
        return NULL;

    LFile file = LCell_GetFile(cell);
    LLayer current = LLayer_GetCurrent(file);
    LLayer layer;
    int capacity = 0;
    for (layer = LLayer_GetList(file); layer != NULL; layer = LLayer_GetNext(layer))
        capacity++;

    if (capacity == 0)
        return NULL;

    LLayer *layers = malloc(capacity * sizeof(*layers));
    char (*names)[256] = malloc(capacity * sizeof(*names));
    const char **options = malloc(capacity * sizeof(*options));
    int count = 0;
    int defaultIndex = 0;
    LLayer result = NULL;
    if (layers != NULL && names != NULL && options != NULL)
    {
        for (layer = LLayer_GetList(file); layer != NULL; layer = LLayer_GetNext(layer))
        {
            LLayerParamEx1512 params;
            if (LLayer_IsDerived(layer) ||
                LLayer_GetParametersEx1512(layer, &params) != LStatusOK ||
                params.Locked || params.Protected)
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

        if (count > 0)
        {
            int picked = LDialog_PickList("Output layer", options, count, defaultIndex);
            if (picked >= 0 && picked < count)
                result = layers[picked];
        }
        else
            LDialog_AlertBox("No writable output layer in the current design.");
    }
    else
        LDialog_AlertBox("Could not load the output layer list.");

    free(options);
    free(names);
    free(layers);
    return result;
}

#endif
