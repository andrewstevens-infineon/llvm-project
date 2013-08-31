//===- lib/ReaderWriter/PECOFF/PECOFFLinkingContext.cpp -------------------===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Atoms.h"
#include "GroupedSectionsPass.h"
#include "IdataPass.h"
#include "LinkerGeneratedSymbolFile.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/Path.h"
#include "lld/Core/InputFiles.h"
#include "lld/Core/PassManager.h"
#include "lld/Passes/LayoutPass.h"
#include "lld/ReaderWriter/PECOFFLinkingContext.h"
#include "lld/ReaderWriter/Reader.h"
#include "lld/ReaderWriter/Simple.h"
#include "lld/ReaderWriter/Writer.h"

namespace lld {

namespace {} // anonymous namespace

error_code PECOFFLinkingContext::parseFile(
    std::unique_ptr<MemoryBuffer> &mb,
    std::vector<std::unique_ptr<File>> &result) const {
  return _reader->parseFile(mb, result);
}

bool PECOFFLinkingContext::validateImpl(raw_ostream &diagnostics) {
  if (_stackReserve < _stackCommit) {
    diagnostics << "Invalid stack size: reserve size must be equal to or "
                << "greater than commit size, but got " << _stackCommit
                << " and " << _stackReserve << ".\n";
    return true;
  }

  if (_heapReserve < _heapCommit) {
    diagnostics << "Invalid heap size: reserve size must be equal to or "
                << "greater than commit size, but got " << _heapCommit
                << " and " << _heapReserve << ".\n";
    return true;
  }

  _reader = createReaderPECOFF(*this);
  _writer = createWriterPECOFF(*this);
  return false;
}

std::unique_ptr<File> PECOFFLinkingContext::createEntrySymbolFile() {
  if (entrySymbolName().empty())
    return nullptr;
  std::unique_ptr<SimpleFile> entryFile(
      new SimpleFile(*this, "command line option /entry"));
  entryFile->addAtom(
      *(new (_allocator) SimpleUndefinedAtom(*entryFile, entrySymbolName())));
  return std::move(entryFile);
}

std::unique_ptr<File> PECOFFLinkingContext::createUndefinedSymbolFile() {
  if (_initialUndefinedSymbols.empty())
    return nullptr;
  std::unique_ptr<SimpleFile> undefinedSymFile(
      new SimpleFile(*this, "command line option /c (or) /include"));
  for (auto undefSymStr : _initialUndefinedSymbols)
    undefinedSymFile->addAtom(*(new (_allocator) SimpleUndefinedAtom(
                                   *undefinedSymFile, undefSymStr)));
  return std::move(undefinedSymFile);
}

void PECOFFLinkingContext::addImplicitFiles(InputFiles &files) const {
  auto *linkerFile = new (_alloc) coff::LinkerGeneratedSymbolFile(*this);
  files.appendFile(*linkerFile);
}

/// Try to find the input library file from the search paths and append it to
/// the input file list. Returns true if the library file is found.
StringRef PECOFFLinkingContext::searchLibraryFile(StringRef filename) const {
  // Current directory always takes precedence over the search paths.
  if (llvm::sys::path::is_absolute(filename) || llvm::sys::fs::exists(filename))
    return filename;
  // Iterate over the search paths.
  for (StringRef dir : _inputSearchPaths) {
    SmallString<128> path = dir;
    llvm::sys::path::append(path, filename);
    if (llvm::sys::fs::exists(path.str()))
      return allocateString(path.str());
  }
  return filename;
}

Writer &PECOFFLinkingContext::writer() const { return *_writer; }

ErrorOr<Reference::Kind>
PECOFFLinkingContext::relocKindFromString(StringRef str) const {
  return make_error_code(yaml_reader_error::illegal_value);
}

ErrorOr<std::string>
PECOFFLinkingContext::stringFromRelocKind(Reference::Kind kind) const {
  return make_error_code(yaml_reader_error::illegal_value);
}

void PECOFFLinkingContext::addPasses(PassManager &pm) const {
  pm.add(std::unique_ptr<Pass>(new pecoff::GroupedSectionsPass()));
  pm.add(std::unique_ptr<Pass>(new pecoff::IdataPass()));
  pm.add(std::unique_ptr<Pass>(new LayoutPass()));
}
} // end namespace lld
