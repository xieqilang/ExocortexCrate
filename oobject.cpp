#include "extension.h"
#include "oarchive.h"
#include "oobject.h"
#include "oproperty.h"
//#include "ixformproperty.h"
#include "foundation.h"
#include <boost/lexical_cast.hpp>

Alembic::Abc::OCompoundProperty getCompoundFromOObject(oObjectPtr in_Casted)
{
   ALEMBIC_TRY_STATEMENT
   switch(in_Casted.mType)
   {
      case oObjectType_Xform:
         return in_Casted.mXform->getSchema();
      case oObjectType_Camera:
         return in_Casted.mCamera->getSchema();
      case oObjectType_PolyMesh:
         return in_Casted.mPolyMesh->getSchema();
      case oObjectType_Curves:
         return in_Casted.mCurves->getSchema();
      case oObjectType_Points:
         return in_Casted.mPoints->getSchema();
      case oObjectType_SubD:
         return in_Casted.mSubD->getSchema();
   }
   return Alembic::Abc::OCompoundProperty();
   ALEMBIC_VALUE_CATCH_STATEMENT(Alembic::Abc::OCompoundProperty())
}

Alembic::Abc::TimeSamplingPtr getTimeSamplingFromObject(Alembic::Abc::OObject object)
{
   ALEMBIC_TRY_STATEMENT
   const Alembic::Abc::MetaData &md = object.getMetaData();
   if(Alembic::AbcGeom::OXform::matches(md)) {
      return Alembic::AbcGeom::OXform(object,Alembic::Abc::kWrapExisting).getSchema().getTimeSampling();
   } else if(Alembic::AbcGeom::OPolyMesh::matches(md)) {
      return Alembic::AbcGeom::OPolyMesh(object,Alembic::Abc::kWrapExisting).getSchema().getTimeSampling();
   } else if(Alembic::AbcGeom::OCurves::matches(md)) {
      return Alembic::AbcGeom::OCurves(object,Alembic::Abc::kWrapExisting).getSchema().getTimeSampling();
   } else if(Alembic::AbcGeom::ONuPatch::matches(md)) {
      return Alembic::AbcGeom::ONuPatch(object,Alembic::Abc::kWrapExisting).getSchema().getTimeSampling();
   } else if(Alembic::AbcGeom::OPoints::matches(md)) {
      return Alembic::AbcGeom::OPoints(object,Alembic::Abc::kWrapExisting).getSchema().getTimeSampling();
   } else if(Alembic::AbcGeom::OSubD::matches(md)) {
      return Alembic::AbcGeom::OSubD(object,Alembic::Abc::kWrapExisting).getSchema().getTimeSampling();
   } else if(Alembic::AbcGeom::OCamera::matches(md)) {
      return Alembic::AbcGeom::OCamera(object,Alembic::Abc::kWrapExisting).getSchema().getTimeSampling();
   }
   return Alembic::Abc::TimeSamplingPtr();
   ALEMBIC_VALUE_CATCH_STATEMENT(Alembic::Abc::TimeSamplingPtr())
}

size_t getNumSamplesFromObject(Alembic::Abc::OObject object)
{
   ALEMBIC_TRY_STATEMENT
   const Alembic::Abc::MetaData &md = object.getMetaData();
   if(Alembic::AbcGeom::OXform::matches(md)) {
      return Alembic::AbcGeom::OXform(object,Alembic::Abc::kWrapExisting).getSchema().getNumSamples();
   } else if(Alembic::AbcGeom::OPolyMesh::matches(md)) {
      return Alembic::AbcGeom::OPolyMesh(object,Alembic::Abc::kWrapExisting).getSchema().getNumSamples();
   } else if(Alembic::AbcGeom::OCurves::matches(md)) {
      return Alembic::AbcGeom::OCurves(object,Alembic::Abc::kWrapExisting).getSchema().getNumSamples();
   } else if(Alembic::AbcGeom::ONuPatch::matches(md)) {
      return Alembic::AbcGeom::ONuPatch(object,Alembic::Abc::kWrapExisting).getSchema().getNumSamples();
   } else if(Alembic::AbcGeom::OPoints::matches(md)) {
      return Alembic::AbcGeom::OPoints(object,Alembic::Abc::kWrapExisting).getSchema().getNumSamples();
   } else if(Alembic::AbcGeom::OSubD::matches(md)) {
      return Alembic::AbcGeom::OSubD(object,Alembic::Abc::kWrapExisting).getSchema().getNumSamples();
   } else if(Alembic::AbcGeom::OCamera::matches(md)) {
      return Alembic::AbcGeom::OCamera(object,Alembic::Abc::kWrapExisting).getSchema().getNumSamples();
   }
   return 0;
   ALEMBIC_VALUE_CATCH_STATEMENT(0)
}

static PyObject * oObject_getIdentifier(PyObject * self, PyObject * args)
{
   ALEMBIC_TRY_STATEMENT
   oObject * object = (oObject*)self;
   if(object->mArchive == NULL)
   {
      PyErr_SetString(getError(), "Archive already closed!");
      return NULL;
   }
   return Py_BuildValue("s",object->mObject->getFullName().c_str());
   ALEMBIC_PYOBJECT_CATCH_STATEMENT
}

static PyObject * oObject_getType(PyObject * self, PyObject * args)
{
   ALEMBIC_TRY_STATEMENT
   oObject * object = (oObject*)self;
   if(object->mArchive == NULL)
   {
      PyErr_SetString(getError(), "Archive already closed!");
      return NULL;
   }
   return Py_BuildValue("s",object->mObject->getMetaData().get("schema").c_str());
   ALEMBIC_PYOBJECT_CATCH_STATEMENT
}

static PyObject * oObject_setMetaData(PyObject * self, PyObject * args)
{
   ALEMBIC_TRY_STATEMENT
   oObject * object = (oObject*)self;
   if(object->mArchive == NULL)
   {
      PyErr_SetString(getError(), "Archive already closed!");
      return NULL;
   }

   // check if we have a string tuple
   // parse the args
   PyObject * metaDataTuple = NULL;
   if(!PyArg_ParseTuple(args, "O", &metaDataTuple))
   {
      PyErr_SetString(getError(), "No metaDataTuple specified!");
      return NULL;
   }
   if(!PyTuple_Check(metaDataTuple) && !PyList_Check(metaDataTuple))
   {
      PyErr_SetString(getError(), "metaDataTuple argument is not a tuple!");
      return NULL;
   }
   size_t nbStrings = 0;
   if(PyTuple_Check(metaDataTuple))
      nbStrings = PyTuple_Size(metaDataTuple);
   else
      nbStrings = PyList_Size(metaDataTuple);

   if(nbStrings != 20)
   {
      PyErr_SetString(getError(), "metaDataTuple doesn't have exactly 20 items!");
      return NULL;
   }

   std::vector<std::string> metaData(nbStrings);
   for(size_t i=0;i<nbStrings;i++)
   {
      PyObject * item = NULL;
      if(PyTuple_Check(metaDataTuple))
         item = PyTuple_GetItem(metaDataTuple,i);
      else
         item = PyList_GetItem(metaDataTuple,i);
      char * itemStr = NULL;
      if(!PyArg_Parse(item,"s",&itemStr))
      {
         PyErr_SetString(getError(), "Some item of metaDataTuple is not a string!");
         return NULL;
      }
      metaData[i] = itemStr;
   }

#ifdef PYTHON_DEBUG
   printf("retrieving ocompound...\n");
   if(object->mObject == NULL)
      printf("what the heck?... NULL pointer?\n");
   printf("object name is: %s\n",object->mObject->getFullName().c_str());
#endif

   Alembic::Abc::OCompoundProperty compound = getCompoundFromOObject(object->mCasted);
#ifdef PYTHON_DEBUG
   printf("ocompound retrieved.\n");
#endif
   if(!compound.valid())
      return Py_BuildValue("i",0);;

#ifdef PYTHON_DEBUG
   printf("creating metadata property...\n");
#endif

   if ( compound.getPropertyHeader( ".metadata" ) != NULL )
   {
      PyErr_SetString(getError(), "Metadata already set!");
      return NULL;
   }

   Alembic::Abc::OStringArrayProperty metaDataProperty = Alembic::Abc::OStringArrayProperty(compound, ".metadata", compound.getMetaData(), compound.getTimeSampling() );

#ifdef PYTHON_DEBUG
   printf("metadata property created.\n");
#endif

   Alembic::Abc::StringArraySample metaDataSample(&metaData.front(),metaData.size());
   metaDataProperty.set(metaDataSample);

   //todo existing properties
   //Alembic::Abc::ArrayPropertyWriterPtr arrayPtr =  boost::static_pointer_cast<Alembic::Abc::ArrayPropertyWriter>(compound.getProperty( ".metadata" ).getPtr());
   //metaDataProperty = Alembic::Abc::OStringArrayProperty(arrayPtr, Alembic::Abc::kWrapExisting);

   return Py_BuildValue("i",1);
   ALEMBIC_PYOBJECT_CATCH_STATEMENT
}

static PyObject * oObject_getProperty(PyObject * self, PyObject * args)
{
   ALEMBIC_TRY_STATEMENT
   oObject * object = (oObject*)self;
   if(object->mArchive == NULL)
   {
      PyErr_SetString(getError(), "Archive already closed!");
      return NULL;
   }

   char * propName = NULL;
   char * propType = NULL;
   int tsIndex = 1;
   if(!PyArg_ParseTuple(args, "s|si", &propName,&propType,&tsIndex))
   {
      PyErr_SetString(getError(), "No property name and/or property type specified!");
      return NULL;
   }

   // special case xform prop
   if(Alembic::AbcGeom::OXform::matches(object->mObject->getMetaData()))
   {
      std::string propNameStr(propName);
      if(propNameStr == ".xform" || propNameStr == ".vals")
      {
         return oXformProperty_new(object->mCasted,object->mArchive,(boost::uint32_t)tsIndex);
      }
   }

   return oProperty_new(object->mCasted,propName,propType,tsIndex, object->mArchive);
   ALEMBIC_PYOBJECT_CATCH_STATEMENT
}

static PyMethodDef oObject_methods[] = {
   {"getIdentifier", (PyCFunction)oObject_getIdentifier, METH_NOARGS},
   {"getType", (PyCFunction)oObject_getType, METH_NOARGS},
   {"setMetaData", (PyCFunction)oObject_setMetaData, METH_VARARGS},
   {"getProperty", (PyCFunction)oObject_getProperty, METH_VARARGS},
   {NULL, NULL}
};
static PyObject * oObject_getAttr(PyObject * self, char * attrName)
{
   return Py_FindMethod(oObject_methods, self, attrName);
}

void oObject_deletePointers(oObject * object)
{
   if(object->mObject != NULL)
   {
      delete(object->mObject);
      object->mObject = NULL;
   }
   if(object->mCasted.mXform != NULL)
   {
      switch(object->mCasted.mType)
      {
         case oObjectType_Xform:
         {
            delete(object->mCasted.mXform);
            object->mCasted.mXform = NULL;
            break;
         }
         case oObjectType_Camera:
         {
            delete(object->mCasted.mCamera);
            object->mCasted.mCamera = NULL;
            break;
         }
         case oObjectType_PolyMesh:
         {
            delete(object->mCasted.mPolyMesh);
            object->mCasted.mPolyMesh = NULL;
            break;
         }
         case oObjectType_Curves:
         {
            delete(object->mCasted.mCurves);
            object->mCasted.mCurves = NULL;
            break;
         }
         case oObjectType_Points:
         {
            delete(object->mCasted.mPoints);
            object->mCasted.mPoints = NULL;
            break;
         }
         case oObjectType_SubD:
         {
            delete(object->mCasted.mSubD);
            object->mCasted.mSubD = NULL;
            break;
         }
      }
   }
}

static void oObject_delete(PyObject * self)
{
   ALEMBIC_TRY_STATEMENT
   // delete the object
   oObject * object = (oObject *)self;
   oObject_deletePointers(object);
   PyObject_FREE(object);
   ALEMBIC_VOID_CATCH_STATEMENT
}

static PyTypeObject oObject_Type = {
  PyObject_HEAD_INIT(&PyType_Type)
  0,                                // op_size
  "oObject",                        // tp_name
  sizeof(oObject),                  // tp_basicsize
  0,                                // tp_itemsize
  (destructor)oObject_delete,       // tp_dealloc
  0,                                // tp_print
  (getattrfunc)oObject_getAttr,     // tp_getattr
  0,                                // tp_setattr
  0,                                // tp_compare
};

PyObject * oObject_new(Alembic::Abc::OObject in_Object, oObjectPtr in_Casted, void * in_Archive)
{
   ALEMBIC_TRY_STATEMENT
   oObject * object = PyObject_NEW(oObject, &oObject_Type);
   if (object != NULL)
   {
#ifdef PYTHON_DEBUG
      printf("creating new oObject from OObject: '%s'\n",in_Object.getFullName().c_str());
#endif PYTHON_DEBUG
      object->mObject = new Alembic::Abc::OObject(in_Object,Alembic::Abc::kWrapExisting);
      object->mCasted = in_Casted;
      object->mArchive = in_Archive;
   }
   return (PyObject *)object;
   ALEMBIC_PYOBJECT_CATCH_STATEMENT
}
