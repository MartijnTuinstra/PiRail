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
    BIT  = 16
    LIST = 32
    NESTED = 64

    def toTypeC(self):
    	types = ['', 'uint8_t', 'uint16_t', 'uint32_t', 'uint64_t', 'int8_t', 'int16_t', 'int32_t', 'int64_t', 
    	         'float', 'double', 'char', 'bool']

    	return types[self.value]


class Structure:
	def __init__(self, filename, name, programStructure, fileStructure):
		self.name = name
		self.structBasename = "configStruct_" + name
		self.filename = filename

		self.pS = programStructure
		self.fS = fileStructure

	def toTypeC(self):
		return f"struct {self.structBasename}"

	def generateCstruct(self):
		s = f"struct configStruct_{self.name}\n{{\n"

		for f in self.pS:
			if isinstance(f, StF):
				if not isinstance(f.type, FieldTypes):
					s += f"  struct {f.type.structBasename} {f.name};\n"

				elif (f.type & FieldTypes.LIST):
					if hasattr(f, "fixedSize"):
						s += f"  {f.nested.toTypeC()} {f.name}[{f.fixedSize}];\n"
					else:
						s += f"  {f.nested.toTypeC()} * {f.name};\n"
				else:
					s += f"  {f.type.toTypeC()} {f.name};\n"

		s += "};\n"
		return s


	def generateCread(self, version):
		header = f"void Config_read_{self.name}_{str(version)}(struct configStruct_{self.name} * obj, uint8_t ** buffer)"
		s = f"{header}{{\n"

		bitFieldTypeCounter = 0;
		bitFieldSame = False;

		fI = 0

		while fI < len(self.fS[version]):
			f   = self.fS[version][fI]
			fI += 1

			if not isinstance(f, FF):
				print("No a File Field ", f)

			elif not isinstance(f.type, (FT, int)):
				s += f"    Config_read_{f.type.name}_{f.version}(&obj->{self.pS[f.id].name}, buffer);\n"

			elif (f.type & FT.LIST and f.type & FT.NESTED):

				s += f"\n    obj->{self.pS[f.id].name} = ({ f.nested.structBasename } *)_calloc" \
				     f"(obj->{self.pS[f.lengthField].name}, struct { f.nested.structBasename });\n"
				
				s += f"    for(unsigned int i = 0; i < obj->{self.pS[f.lengthField].name}; i++){{\n"
				s += f"        Config_read_{ f.nested.name }_{ f.version }(&obj->{ self.pS[f.id].name }[i], buffer);\n"
				s += f"    }}\n"

			elif (f.type & FieldTypes.LIST):
				FieldType = FT(f.type & 0x0F).toTypeC()

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

				if not bitFieldSame:
					bitFieldTypeCounter += 1
					tempName = f"tmp{bitFieldTypeCounter}"

					s += f"\n    {bitFieldType} {tempName} = 0;\n"
					s += f"    Config_read_{bitFieldType}_{bitFieldType}(&{tempName}, buffer);\n"

				s += f"    Config_read_BitField(&obj->{self.pS[f.id].name}, {tempName}, {f.offset}, {f.width});\n"

				nF = self.fS[version][fI]

				if(nF.type & FT.BIT and bitFieldType == FT(nF.type & 0x0F).toTypeC()):
					bitFieldSame = True
				else:
					bitFieldSame = False
					s += "\n"

			else:
				s += f"    Config_read_{f.type.toTypeC()}_{self.pS[f.id].type.toTypeC()}(&obj->{self.pS[f.id].name}, buffer);\n"
			# lengthField

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
				code['write'] += f"    for(unsigned int i = 0; i < obj->{ self.pS[f.lengthField].name }; i++){{\n"
				code['write'] += f"        Config_write_{f.nested.name}(&obj->{self.pS[f.id].name}[i], buffer);\n"
				code['write'] += f"    }}\n"

				code['size'] += f"    for(unsigned int i = 0; i < obj->{self.pS[f.lengthField].name}; i++){{\n"
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

				if not bitFieldSame:
					bitFieldTypeCounter += 1
					tempName = f"tmp{bitFieldTypeCounter}"

					code['write'] += f"\n    {bitFieldType} {tempName} = 0;\n"

				code['write'] += f"    Config_write_BitField(obj->{self.pS[f.id].name}, &{tempName}, {f.offset}, {f.width});\n"

				nF = self.fS[version][fI]

				if(nF.type & FT.BIT and bitFieldType == FT(nF.type & 0x0F).toTypeC() and f.type != 0):
					bitFieldSame = True
				else:
					bitFieldSame = False
					code['write'] += f"    Config_write_{bitFieldType}(&{tempName}, buffer); // BitField\n\n"
					code['size']  += f"    size += sizeof({bitFieldType});\n"

			else:
				code['write'] += f"    Config_write_{f.type.toTypeC()}(&obj->{self.pS[f.id].name}, buffer);\n"
				code['size']  += f"    size += sizeof({f.type.toTypeC()});\n"


		code['write'] += "}\n"
		code['size']  += "    return size;\n}\n"

		return header, code


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

class FF: # File Field
	def __init__(self, type, id, **kwargs):
		self.type = type
		self.id = id

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

	def writeToFile(self):
		baseDir = Path(os.getcwd())
		headerFileName = baseDir / self.headerPath / f"{self.filename}.h"
		sourceFileName = baseDir / self.sourcePath / f"{self.filename}.cpp"

		text = [t.generate() for t in self.structures.values()]
		output = ([t[0] for t in text], ["\n".join(t[1]) for t in text], ["\n".join(t[2]) for t in text])

		LUtable = f"\n\nuint8_t Config_{self.filename}LU[{len(self.vLU)}][{len(self.vLU[0])}] = {str(self.vLU).replace('[', '{').replace(']', '}')};\n"
		LUtableH = f"\n\nextern uint8_t Config_{self.filename}LU[{len(self.vLU)}][{len(self.vLU[0])}];\n"

		l = []
		for t in self.structures.keys():
			l += [f"#define CONFIG_{self.filename.upper()}_LU_{t.upper()} {len(l)}"]


		LUtableH += "\n".join(l)
		LUtableH += f"\n#define CONFIG_{self.filename.upper()}_LU_MAX_VERSION {len(self.vLU) - 1}"
		LUtableH += "\n"

		hf = open(str(headerFileName), 'w')
		hf.write(f"#ifndef INCLUDE_CONFIG_{self.filename.upper()}_H\n")
		hf.write(f"#define INCLUDE_CONFIG_{self.filename.upper()}_H\n")
		hf.write("#include <stdlib.h>\n#include <stdint.h>\n\n")
		hf.write("\n".join(output[0]))
		hf.write(self.extraStructures)
		hf.write("\n".join(output[2]))
		hf.write(LUtableH)
		hf.write("\n#endif\n")
		hf.close()

		cf = open(str(sourceFileName), 'w')
		cf.write(f"#include \"{self.includePath}configReader.h\"\n")
		cf.write(f"#include \"{self.includePath}{self.filename}.h\"\n\n")
		cf.write("\n".join(output[1]))
		cf.write(LUtable)
		cf.close()


