import enum
import time
import os

from pathlib import Path

class FieldTypes(enum.IntEnum):
    U8  = 1
    U16 = 2
    U32 = 3
    U64 = 4
    I8  = 5
    I16 = 6
    I32 = 7
    I64 = 8
    FL32 = 9
    FL64 = 10
    CHAR = 11
    UNION = 12
    BOOL = 13
    BIT  = 16
    LIST = 32
    NESTED = 64

    def toTypeC(self):
        types = ['', 'uint8_t', 'uint16_t', 'uint32_t', 'uint64_t', 'int8_t', 'int16_t', 'int32_t', 'int64_t', 
                 'float', 'double', 'char', 'union', 'bool']

        return types[self.value]


class Structure:
    def __init__(self, filename, name, programStructure, fileStructure, Scannable=None, Preview=None, Editable=None, **kwargs):
        self.name = name
        self.structBasename = "configStruct_" + name
        self.filename = filename

        temp = {f.id: f for f in programStructure}
        self.pS = [temp[key] if key in temp else None for key in range(min(temp.keys()), max(temp.keys())+1)]

        self.fS = fileStructure

        self.Scannable = Scannable
        self.Preview = Preview
        self.Editable = Editable
        
        for key, value in kwargs.items():
            setattr(self, key, value)

    def toTypeC(self):
        return f"struct {self.structBasename}"

    def generateCstruct(self):
        s = f"struct configStruct_{self.name}\n{{\n"

        for f in self.pS:
            if isinstance(f, StF):
                if not isinstance(f.type, (FieldTypes, int)):
                    s += f"  struct {f.type.structBasename} {f.name};\n"

                elif (f.type & FieldTypes.LIST):
                    if hasattr(f, "fixedSize"):
                        s += f"  {f.nested.toTypeC()} {f.name}[{f.fixedSize}];\n"
                    else:
                        s += f"  {f.nested.toTypeC()} * {f.name};\n"
                elif (f.type & FieldTypes.BIT):
                    s += f"  {FieldTypes(f.type & ~(FieldTypes.BIT)).toTypeC()} {f.name}:{f.bitsize};\n"
                elif (f.type == FieldTypes.UNION):
                    s += f"  union {{\n    {f.size.toTypeC()} raw;\n    struct {{\n      {f.unionStruct}\n    }} data;\n  }} {f.name};\n"
                else:
                    s += f"  {f.type.toTypeC()} {f.name};\n"

        s += "};\n"
        return s


    def generateCread(self, version):
        header = f"void Config_read_{self.name}_{str(version)}(struct configStruct_{self.name} * obj, uint8_t ** buffer)"
        s = f"{header}{{\n"

        bitFieldTypeCounter = 0
        bitFieldSame = False

        fI = 0

        openFields = [f.id for f in self.pS if f is not None]
        print(self.name, openFields)

        while fI < len(self.fS[version]):
            f   = self.fS[version][fI]
            fI += 1

            if isinstance(f.id, list):
                for id in f.id:
                    openFields.remove(id)
            elif f.id in openFields:
                print(f.id)
                openFields.remove(f.id)
            else:
                raise ValueError("Multiple File Field with the same id", f.id)

            if not isinstance(f, FF):
                print("No a File Field ", f)

            elif not isinstance(f.type, (FT, int)):
                s += f"    Config_read_{f.type.name}_{f.version}(&obj->{self.pS[f.id].name}, buffer);\n"

            elif (f.type & FT.LIST and f.type & FT.NESTED):
                loopCounter = ""

                if hasattr(self.pS[f.id], "fixedSize"):
                    loopCounter = f"{self.pS[f.id].fixedSize}"
                elif hasattr(f, "lengthField"):
                    s += f"\n    obj->{self.pS[f.id].name} = ({ f.nested.structBasename } *)_calloc" \
                        f"(obj->{self.pS[f.lengthField].name}, struct { f.nested.structBasename });\n"
                    loopCounter = f"obj->{self.pS[f.lengthField].name}"
                else:
                    continue
                
                s += f"    for(unsigned int i = 0; i < {loopCounter}; i++){{\n"
                s += f"        Config_read_{ f.nested.name }_{ f.version }(&obj->{ self.pS[f.id].name }[i], buffer);\n"
                s += f"    }}\n"

            elif (f.type & FieldTypes.LIST):
                print(f"reading {f.type}")
                print(f"")
                FieldType = FT(f.type & ~(FT.LIST)).toTypeC()

                if hasattr(f, "sizeMultiply"):
                    sizeMultiply = f.sizeMultiply
                else:
                    sizeMultiply = 1


                if hasattr(self.pS[f.id], "fixedSize"):
                    s += f"    memcpy(obj->{self.pS[f.id].name}, *buffer, {self.pS[f.id].fixedSize * sizeMultiply} * sizeof({FieldType}));\n"
                    s += f"    *buffer += sizeof({FieldType}) * {self.pS[f.id].fixedSize * sizeMultiply};\n"
                elif hasattr(f, "lengthField"):
                    len_field = self.pS[f.lengthField].name

                    extraSize = 0
                    if hasattr( self.pS[f.id], "nested") and self.pS[f.id].nested == FT.CHAR:
                        extraSize = 1

                    s += f"\n    obj->{self.pS[f.id].name} = ({FieldType} *)_calloc(obj->{len_field} * {sizeMultiply} + {extraSize}, {FieldType});\n"
                    s += f"    memcpy(obj->{self.pS[f.id].name}, *buffer, obj->{len_field} * {sizeMultiply} * sizeof({FieldType}));\n"
                    s += f"    *buffer += sizeof({FieldType}) * {sizeMultiply} * obj->{len_field};\n"
                    
                # else:
                # 	s += f"\n    obj->{self.pS[f.id].name} = ({FieldType} *)_calloc({f.size}, {FieldType});\n"
                # 	s += f"    memcpy(obj->{self.pS[f.id].name}, *buffer, {f.size} * sizeof({FieldType}));\n"
                # 	s += f"    *buffer += sizeof({FieldType}) * {f.size};\n"
            elif (f.type & FT.BIT):
                bitFieldType = FT(f.type & 0x0F).toTypeC()
                tempName = f"tmp{bitFieldTypeCounter}"
                bitFieldTypeCounter += 1

                s += f"\n    {bitFieldType} {tempName} = 0;\n"
                s += f"    Config_read_{bitFieldType}_{bitFieldType}(&{tempName}, buffer);\n"

                for i in range(0, len(f.id)):
                    s += f"    Config_read_BitField(&obj->{self.pS[f.id[i]].name}, {tempName}, {f.offset[i]}, {f.width[i]});\n"

                s += "\n"

            elif self.pS[f.id].type == FT.UNION:
                s += f"    Config_read_{f.type.toTypeC()}_{self.pS[f.id].size.toTypeC()}(&obj->{self.pS[f.id].name}.raw, buffer);\n"
            else:
                s += f"    Config_read_{f.type.toTypeC()}_{self.pS[f.id].type.toTypeC()}(&obj->{self.pS[f.id].name}, buffer);\n"
            # lengthField

        s += "\n";

        for of in openFields:
            f = self.pS[of]
            if(hasattr(f, "default")):
                s += f"    obj->{f.name} = {f.default};\n"

        s += "}\n"

        return (header + ";\n"), s


    def generateCwrite(self):
        header = {}
        header['write'] = f"void Config_write_{self.name}(struct configStruct_{self.name} * obj, uint8_t ** buffer);\n"
        header['size']  = f"int Config_write_size_{self.name}(struct configStruct_{self.name} * obj);\n"
        
        code = {}
        code['write'] = f"{header['write'][:-2]}{{\n"
        code['size']  = f"{header['size'][:-2]}{{\n    int size = 0;\n"

        bitFieldTypeCounter = 0;
        bitFieldSame = False;

        version = len(self.fS) - 1

        fI = 0

        while fI < len(self.fS[version]):
            f   = self.fS[version][fI]
            fI += 1

            # if isinstance(f, FF):
            # 	pass

            if not isinstance(f.type, (FT, int)):
                code['write'] += f"    Config_write_{f.type.name}(&obj->{self.pS[f.id].name}, buffer);\n"
                code['size']  += f"    size += Config_write_size_{f.type.name}(&obj->{self.pS[f.id].name});\n"

            elif (f.type & FT.LIST and f.type & FT.NESTED):
                loopCounter = ""
                if hasattr(self.pS[f.id], "fixedSize"):
                    loopCounter = f"{self.pS[f.id].fixedSize}"
                elif hasattr(f, "lengthField"):
                    loopCounter = f"obj->{ self.pS[f.lengthField].name }"

                code['write'] += f"    for(unsigned int i = 0; i < { loopCounter }; i++){{\n"
                code['write'] += f"        Config_write_{f.nested.name}(&obj->{self.pS[f.id].name}[i], buffer);\n"
                code['write'] += f"    }}\n"

                code['size'] += f"    for(unsigned int i = 0; i < { loopCounter }; i++){{\n"
                code['size'] += f"        size += Config_write_size_{f.nested.name}(&obj->{self.pS[f.id].name}[i]);\n"
                code['size'] += f"    }}\n"

            elif (f.type & FieldTypes.LIST):
                FieldType = FT(f.type & 0x0F).toTypeC()

                if hasattr(f, "sizeMultiply"):
                    sizeMultiply = f.sizeMultiply
                else:
                    sizeMultiply = 1


                if hasattr(self.pS[f.id], "fixedSize"):
                    code['write'] += f"\n    if (obj->{self.pS[f.id].name})\n"
                    code['write'] += f"         memcpy(*buffer, obj->{self.pS[f.id].name}, {self.pS[f.id].fixedSize * sizeMultiply} * sizeof({FieldType}));\n"
                    code['write'] += f"    *buffer += sizeof({FieldType}) * {self.pS[f.id].fixedSize * sizeMultiply};\n"

                    code['size']  += f"    size += {self.pS[f.id].fixedSize * sizeMultiply} * sizeof({FieldType});\n"
                else:
                    code['write'] += f"\n    if (obj->{self.pS[f.id].name})\n"
                    code['write'] += f"        memcpy(*buffer, obj->{self.pS[f.id].name}, obj->{ self.pS[f.lengthField].name } * {sizeMultiply} * sizeof({FieldType}));\n"
                    code['write'] += f"    *buffer += sizeof({FieldType}) * obj->{ self.pS[f.lengthField].name } * {sizeMultiply};\n"

                    code['size']  += f"    size += sizeof({FieldType}) * obj->{self.pS[f.lengthField].name} * {sizeMultiply};\n"
                # 	if f[2] == -1:
                # 		s += f"\n    memcpy(*buffer, obj->{self.pS[f.id].name}, {f[3]} * sizeof({FieldType}));\n"
                # 		s += f"    *buffer += sizeof({FieldType}) * {f[3]};\n"
                # else:
                # continue

            elif (f.type & FT.BIT):
                bitFieldType = FT(f.type & 0x0F).toTypeC()
                tempName = f"tmp{bitFieldTypeCounter}"
                bitFieldTypeCounter += 1

                code['write'] += f"\n    {bitFieldType} {tempName} = 0;\n"

                for i in range(0, len(f.id)):
                    code['write'] += f"    Config_write_BitField(obj->{self.pS[f.id[i]].name}, &{tempName}, {f.offset[i]}, {f.width[i]});\n"

                code['write'] += f"    Config_write_{bitFieldType}(&{tempName}, buffer); // BitField\n\n"
                code['size']  += f"    size += sizeof({bitFieldType});\n"

            elif self.pS[f.id].type == FT.UNION:
                code['write'] += f"    Config_write_{f.type.toTypeC()}(&obj->{self.pS[f.id].name}.raw, buffer);\n"
                code['size']  += f"    size += sizeof({f.type.toTypeC()});\n"
            else:
                code['write'] += f"    Config_write_{f.type.toTypeC()}(&obj->{self.pS[f.id].name}, buffer);\n"
                code['size']  += f"    size += sizeof({f.type.toTypeC()});\n"


        code['write'] += "}\n"
        code['size']  += "    return size;\n}\n"

        return header, code

    def generateEditorHeader(self):
        if self.Preview is not None and self.Preview:
            s  = f"void configEditor_preview_{self.name}(char *, struct configStruct_{self.name});\n"
        if self.Scannable is not None and self.Scannable:
            s += f"void configEditor_scan_{self.name}(char *, struct configStruct_{self.name} *);\n"
        return s

    def generateEditorPreview(self):
        if self.Preview is None or not self.Preview:
            return

        c = self.Preview

        s  = f"void configEditor_preview_{self.name}(char * buffer, struct configStruct_{self.name} data){{\n"
        s += f"  sprintf(buffer, \"({c.pF})\", "

        scanFields = [('data.'+self.pS[f].name) for f in c.f]
        if hasattr(c, 'textIndex'):
            for i in range(len(c.f)):
                if c.textIndex[i] is None:
                    continue

                scanFields[i] = f"{c.textIndex[i]}[{scanFields[i]}]"

        s +=   f"{', '.join(scanFields)});\n"

        s += "}\n"

        return s

    def generateEditorScan(self):
        print("generateEditorScan")
        if self.Scannable is None or not self.Scannable:
            return
        
        s  = f"void configEditor_scan_{self.name}(char * buffer, struct configStruct_{self.name} * data){{\n"

        c = self.Scannable

        if not hasattr(self, "Scannable"):
            print('not scannable')
            return
        if not self.Scannable:
            return

        scanFieldTypes = []

        l = 0
        mode = False
        while l < len(c.sF):
            if mode:
                if c.sF[l] in ['f', 'e', 'E']:
                    scanFieldTypes[-1] = "double"
                    mode = False
                elif c.sF[l] in ['d', 'i', 'u']:
                    scanFieldTypes[-1] += "int"
                    mode = False
                elif c.sF[l] == 'l':
                    scanFieldTypes[-1] += "long "
                elif c.sF[l] == '*':
                    scanFieldTypes.pop(-1)
                    mode = False
            elif c.sF[l] == '%':
                scanFieldTypes += [""]
                mode = True

            l += 1

        print(c.sF, scanFieldTypes)
        # scanFieldTypes = [self.pS[f].type.toTypeC() for f in c.f if self.pS[f] > FieldTypes.I64 else "int"]
        scanFieldNames = [self.pS[c.f[i]].name for i in range(len(c.f))]

        s += "  " + "\n  ".join([f"{scanFieldTypes[i]} t_{scanFieldNames[i]};" for i in range(len(c.f))]) + "\n"

        scanArgs = ', '.join([f"&t_{t}" for t in scanFieldNames])
        s += f"  if(sscanf(buffer, \"{c.sF}\", {scanArgs}) > {len(c.f)-1}){{\n"
        for i in range(len(c.f)):
            s += f"    data->{scanFieldNames[i]} = t_{scanFieldNames[i]};\n"
        # link->module = tmp[0];
        # link->id = tmp[1];
        # link->type = tmp[2];
        # }

        s += "  }\n}\n"

        return s

    def generateEditor(self):
        print("generateEditor")
        if self.Editable is None or not self.Editable:
            print('not editable')
            return

        if len(self.Editable) == 0:
            return
        
        
        arguments = [f"struct configStruct_{self.name} * data"]
        for E in self.Editable:
            if hasattr(E, 'extraArgument'):
                arguments += E.extraArgument

        s  = f'void configEditor_{self.name}({", ".join(arguments)}){{\n'
        s += f"  char scanBuffer[100];\n"
        s += f"  char fieldName[{ConfigHeaderLength + 25}];\n"
        s += f"  char fieldPreview[{ConfigPreviewLength + 25}];\n\n"

        for c in self.Editable:
            if len(c.f) > 1:
                continue
            elif len(c.f) == 0:
                # Print statement only
                s += f'  printf(\"{c.name}\\n\", {", ".join(c.printArgument)});\n'
                continue

            s += f"  scanBuffer[0] = 0;\n"

            if hasattr(c, 'conditional'):
                condition = []
                state = 0
                for i in c.conditional:
                    if state == 0:
                        condition = condition + [f"data->{self.pS[i].name}"]
                        state += 1
                    elif state == 1:
                        if i in ['>', '<', '>=', '<=', "==", "!="]:
                            condition = condition + [i]
                            state += 1
                        else:
                            condition = condition + ["&&", f"data->{self.pS[i].name}"]
                    elif state == 2:
                        if isinstance(i, str):
                            condition = condition + [i]
                        else:
                            condition = condition + [f"data->{self.pS[i].name}"]

                s += f'  if({" ".join(condition)}){{\n'
            else:
                setattr(c, 'conditional', False)

            if hasattr(c, 'custom'):
                s += c.custom
                if c.conditional is not False:
                    s += "  }\n"
                continue

            s += f"  sprintf(fieldName, \"%.{ConfigHeaderLength}s\", \"{c.name}\");\n"

            f = self.pS[c.f[0]]
            t = ""
            listI = ""
            if hasattr(c, 'overrideType'):
                t = c.overrideType
            elif not isinstance(f.type, (FieldTypes, int)):
                t = f.type.name
            elif (f.type & FieldTypes.LIST):
                if isinstance(f.nested, (FieldTypes, int)):
                    t = f"{f.nested.toTypeC()}"
                else:
                    t = f"{f.nested.name}"
                listI = f"data->{self.pS[c.listLength].name}"
            elif (f.type & FieldTypes.BIT):
                t = f"{FieldTypes(f.type & ~(FieldTypes.BIT)).nested.toTypeC()}"
            # elif (f.type == FieldTypes.UNION):
            #     s += f"  union {{\n    {f.size.toTypeC()} raw;\n    struct {{\n      {f.unionStruct}\n    }} data;\n  }} {f.name};\n"
            else:
                t = f.type.toTypeC()

            pn = sn = f"data->{f.name}"
            if listI != "":
                pn += "[i]"
                sn += "[i]"
                s += f"  for(uint8_t i = 0; i < {listI}; i++){{\n"

            pt = st = t
            if hasattr(c, 'textIndex'):
                pn = f"{c.textIndex[0]}[{pn}]"
                pt = "string"
                st = "uint8_t"
            
            if t == "string":
                sn += f", &data->{self.pS[c.listLength].name}"

            s += f"  configEditor_preview_{pt}(fieldPreview, {pn});\n"

            if listI != "":
                s += f"  printf(\"%-{ConfigHeaderLength-3}s %2i %-{ConfigPreviewLength}s | \", fieldName, i, fieldPreview);\n"
            else:
                s += f"  printf(\"%-{ConfigHeaderLength}s %-{ConfigPreviewLength}s | \", fieldName, fieldPreview);\n"

            s += f"  fgets(scanBuffer, 100, stdin);\n"
            s += f"  configEditor_scan_{st}(scanBuffer, &{sn});\n\n"

            if hasattr(c, 'alloc'):
                alloc = c.alloc
                if not isinstance(alloc, list):
                    alloc = [alloc]

                for a in alloc:
                    ptr = f"data->{self.pS[a[0]].name}"
                    s += f"  if ({ptr})\n"
                    s += f"    {ptr} = ({a[1]} *)_realloc({ptr}, {sn}, {a[1]});\n"
                    s += f"  else\n"
                    s += f"    {ptr} = ({a[1]} *)_calloc({sn}, {a[1]});\n\n"

            if listI != "":
                s += "  }\n"
            if c.conditional is not False:
                s += "  }\n"

        return s + "\n}\n"

        c = self.config[0]

        if not hasattr(self, "Preview"):
            return ""


        scanFields = [('data->'+self.pS[f].name) for f in c.f]
        if hasattr(c, 'textIndex'):
            for i in range(len(c.f)):
                if c.textIndex[i] is None:
                    continue

                scanFields[i] = f"{c.textIndex[i]}[{scanFields[i]}]"

        s +=   f"{', '.join(scanFields)});\n"

        # s += f"  {{\n    char buffer[100];\n"
        # s += f"    fgets(buffer, 100, stdin);\n"
        # s += f"    configEditor_scan_{self.name}(buffer, data);\n  }}\n"

        s += "}\n"

# int tmp[3];
#   if(fgetScanf("%i%*c%i%*c%i", &tmp[0], &tmp[1], &tmp[2]) > 2){
#     link->module = tmp[0];
#     link->id = tmp[1];
#     link->type = tmp[2];
#   }


        s += f"  uint8_t field[{ConfigHeaderLength + ConfigPreviewLength + 5}];\n"
        s += f"  sprintf(field, \"%.{ConfigHeaderLength}s\");\n"
        s += f"  sprintf(&field[{ConfigHeaderLength}], \"({c.pF})\", "
        s += "}\n"

    # def previewFunction(self, name=""):
    #     text = f"ConfigEditor_Preview_{self.name}("
    #     return ""
    # def scanFunction(self):
    #     return ""

    def generate(self):

        functionHList = f"void (*Config_read_{self.name}_list[{len(self.fS)}]) (struct configStruct_{self.name} *, uint8_t **)"
        functionList = f"{functionHList} = {{\n    "
        functionList += ",\n    ".join([f"&Config_read_{self.name}_{i}" for i in range(0, len(self.fS))])
        functionList += "\n};\n"
        functionHList = "extern " + functionHList + ";\n"
        functionHList += (f"#define Config_read_{self.name}(_V, _H, _B) Config_read_{self.name}_list[Config_{self.filename}LU" +
                          f"[_V][CONFIG_{self.filename.upper()}_LU_{self.name.upper()}]](_H, _B);\n")

        structure = self.generateCstruct();

        f = [self.generateCread(i) for i in range(0, len(self.fS))]
        f.append((functionHList, functionList))

        writeHeader, writeCode = self.generateCwrite()
        f.append( (f"{writeHeader['write']}\n{writeHeader['size']}", f"{writeCode['write']}\n{writeCode['size']}") )

        header, functions = ([t[0] for t in f], [t[1] for t in f])


        return structure, functions, header

class StF: # Structure Field
    def __init__(self, id, name, type, **kwargs):
        self.id = id
        self.name = str(name)
        self.type = type

        for key, value in kwargs.items():
            setattr(self, key, value)

    def __repr__(self):
        return f"StF<{self.id}, {self.type.name}>"

class FF: # File Field
    def __init__(self, type, id, **kwargs):
        self.type = type
        self.id = id

        for key, value in kwargs.items():
            setattr(self, key, value)
            
        if isinstance(self.id, list) and (self.type & FT.BIT):
            assert len(self.id) == len(self.offset) == len(self.width)

ConfigHeaderLength  = 20
ConfigPreviewLength = 12
class ConfigEntry:
    def __init__(self, fields, name="", **kwargs):
        self.name = name
        self.f  = fields
        
        for key, value in kwargs.items():
            setattr(self, key, value)
class ScanEntry:
    def __init__(self, scanFormat, fields, name="", **kwargs):
        self.name = name
        self.sF = scanFormat
        self.f  = fields
        
        for key, value in kwargs.items():
            setattr(self, key, value)

class PreviewEntry:
    def __init__(self, previewFormat, fields, name="", **kwargs):
        self.name = name
        self.pF = previewFormat
        self.f  = fields
        
        for key, value in kwargs.items():
            setattr(self, key, value)


FT = FieldTypes


class ConfigFileLayout:
    structures = {}
    extraStructures = ""
    includePath = ""
    filePath = ""
    headerPath = ""

    def __init__(self, filename, versionLU, **kwargs):
        self.filename = filename
        self.vLU = versionLU

        for key, value in kwargs.items():
            setattr(self, key, value)

    def addStructure(self, *args, **kwargs):
        structure = Structure(self.filename, *args, **kwargs)
        self.structures[structure.name] = structure
        return structure

    def addCustomStructure(self, structure):
        self.extraStructures += structure

    def writeStructureHeaderFile(self, location, text):
        l = []
        for t in self.structures.keys():
            l += [f"#define CONFIG_{self.filename.upper()}_LU_{t.upper()} {len(l)}"]

        LUtableH = f"\n\nextern uint8_t Config_{self.filename}LU[{len(self.vLU)}][{len(self.vLU[0])}];\n"
        LUtableH += "\n".join(l)
        LUtableH += f"\n#define CONFIG_{self.filename.upper()}_LU_MAX_VERSION {len(self.vLU) - 1}"
        LUtableH += "\n"

        
        hf = open(str(location), 'w')
        hf.write(f"#ifndef INCLUDE_CONFIG_{self.filename.upper()}_H\n")
        hf.write(f"#define INCLUDE_CONFIG_{self.filename.upper()}_H\n")
        hf.write("#include <stdlib.h>\n#include <stdint.h>\n\n")
        hf.write("\n".join(text[0]))
        hf.write(self.extraStructures)
        hf.write("\n".join(text[1]))
        hf.write(LUtableH)
        hf.write("\n#endif\n")
        hf.close()

    def writeStructureSourceFile(self, location, text):
        LUtable = f"\n\nuint8_t Config_{self.filename}LU[{len(self.vLU)}][{len(self.vLU[0])}] = {str(self.vLU).replace('[', '{').replace(']', '}')};\n"

        cf = open(str(location), 'w')
        cf.write(f"#include \"{self.includePath}configReader.h\"\n")
        cf.write(f"#include \"{self.includePath}{self.filename}.h\"\n\n")
        cf.write("\n".join(text))
        cf.write(LUtable)
        cf.close()
    
    def writeConfigEditorHeaderFile(self, location):
        hf = open(str(location), 'w')
        hf.write(f"#ifndef INCLUDE_CONFIG_LAYOUTEDITOR_H\n")
        hf.write(f"#define INCLUDE_CONFIG_LAYOUTEDITOR_H\n")
        hf.write("#include <stdlib.h>\n#include <stdint.h>\n#include \"flags.h\"\n")
        
        text = []
        for k in self.structures:
            s = self.structures[k]

            if hasattr(s, 'Preview') and s.Preview:
                text = text + [f"void configEditor_preview_{s.name}(char *, struct configStruct_{s.name});"]
            if hasattr(s, 'Scannable') and s.Scannable:
                text = text + [f"void configEditor_scan_{s.name}(char *, struct configStruct_{s.name} *);"]
            if hasattr(s, 'Editable') and s.Editable:
                arguments = [f"struct configStruct_{s.name} *"]
                for E in s.Editable:
                    if hasattr(E, 'extraArgument'):
                        arguments += E.extraArgument
                text = text + [f'void configEditor_{s.name}({", ".join(arguments)});']

        hf.write("\n".join([t for t in text if t is not None]))

        hf.write("\n#endif\n")
        hf.close()


    def writeConfigEditorSourceFile(self, location):
        
        text = []
        for k in self.structures:
            s = self.structures[k]
            text = text + [s.generateEditorPreview(), s.generateEditorScan(), s.generateEditor()]

        print(text)

        cf = open(str(location), 'w')
        cf.write(f"#include \"{self.includePath}configReader.h\"\n")
        cf.write(f"#include \"{self.includePath}{self.filename}Editor.h\"\n")
        cf.write(f"#include \"{self.includePath}{self.filename}.h\"\n\n")
        cf.write(f"#include \"utils/strings.h\"\n")
        cf.write("\n".join([t for t in text if t is not None]))
        cf.close()

    def writeToFile(self):
        baseDir = Path(os.getcwd())
        
        structureText = [t.generate() for t in self.structures.values()]

        self.writeStructureHeaderFile(baseDir / self.headerPath / f"{self.filename}.h",  ([t[0] for t in structureText], ["\n".join(t[2]) for t in structureText]))
        self.writeStructureSourceFile(baseDir / self.sourcePath / f"{self.filename}.cpp", ["\n".join(t[1]) for t in structureText])

        self.writeConfigEditorHeaderFile(baseDir / self.headerPath / f"{self.filename}Editor.h")
        self.writeConfigEditorSourceFile(baseDir / self.sourcePath / f"{self.filename}Editor.cpp")



