/*
 * Modifiy_Module
 * Modifiy_Module Marco for Ledit
 *
 * Copyright (c) 2024 muyeyifeng
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
module Modifiy_Module
{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <math.h>
    #include "Ledit_Array.h"

    void Modifiy_Circle(LObject circle, long target_radius)
    {
        LCell Cell_Now = LCell_GetVisible();
		LLayer LLayer_Now = LObject_GetLayer(Cell_Now, circle);

		LPoint center = LCircle_GetCenter(circle);
		LCoord r = LCircle_GetRadius(circle);
        if(r > target_radius)
        {
            LCircle_Set(Cell_Now, circle, center, target_radius);
        }
    }

    void Modifiy_Selected_Object()
    {
        LCell Cell_Now = LCell_GetVisible();
        //****************************Input Params****************************//
		LDialogItem Dialog_Items[2] = {{"Target radius (um)", "0"},
									   {"Keep Original Objects (1 = yes, 0 = no)", "1"}};
		long tar_radius;
		int keep;
		if (LDialog_MultiLineInputBox("Modifiy Selected Object", Dialog_Items, 2))
		{
			tar_radius = (long)(atof(Dialog_Items[0].value) * 1000); // get the tar_radius
			keep = atoi(Dialog_Items[1].value);					 // get the keep
		}
		else
		{
			return;
		}
		//****************************Input Params****************************//
        LSelection selectedInital = LSelection_GetList();
		while (selectedInital != NULL)
		{
			LObject selectedObject = LSelection_GetObject(selectedInital);
            switch(LObject_GetShape(selectedObject))
            {
                case 1: // LCircle
                    Modifiy_Circle(selectedObject, tar_radius);
                    break;
                default:
                break;
            }
            if(keep!=1)
            {
                LObject_Delete(Cell_Now, selectedObject);
            }
			selectedInital = LSelection_GetNext(selectedInital);
		}
        LDisplay_Refresh();
    }

    void Modifiy_func(void)
	{
		LMacro_Register("Modifiy_Selected_Object_func", "Modifiy_Selected_Object");
	}
} /* end of module Array_Module */

Modifiy_func();