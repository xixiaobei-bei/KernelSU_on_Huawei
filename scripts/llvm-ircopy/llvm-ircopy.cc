/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: LLVM function section changer
 * This is unpublished work. See README file for more information.
 * Author: Dmitry Kasatkin
 * Creator: security-ap
 * Date: 2020/04/15
 */

#include "llvm/IRReader/IRReader.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"

using namespace std;
using namespace llvm;

namespace {
    cl::OptionCategory IRCCat("llvm-ircopy options");

    cl::opt<string> InputFilename(cl::Positional, cl::Required, cl::cat(IRCCat),
                                  cl::desc("<input file>"));

    cl::opt<string> OutputFilename("o", cl::Optional,
                                   cl::desc("Output filename"),
                                   cl::value_desc("filename"),
                                   cl::cat(IRCCat));

    cl::opt<string> SectionPrefix("section-prefix", cl::Optional,
                                   cl::desc("function section prefix"),
                                   cl::value_desc("prefix"),
                                   cl::cat(IRCCat));

    cl::opt<bool> Quiet("quiet", cl::desc("Hide output"),
            cl::cat(IRCCat));
    cl::alias Quiet2("q", cl::desc("Alias for --quiet"),
            cl::aliasopt(Quiet), cl::Grouping);

    bool HadError = true;

    string ToolName;

    raw_ostream* out;
} // namespace

static void error(Twine Message, Twine Path = Twine())
{
    HadError = true;
    WithColor::error(errs(), ToolName) << Path << ": " << Message << ".\n";
}

static bool error(error_code EC, Twine Path = Twine())
{
    if (EC) {
        error(EC.message(), Path);
        return true;
    }
    return false;
}

static void setFunctionSection(const string& Filename)
{
    LLVMContext Context;
    SMDiagnostic Err;
    std::unique_ptr<Module> M = parseIRFile(Filename, Err, Context);
    if (!M) {
        errs() << "Parsing error: " << Filename << "\n";
        return;
    }

    Module& Mod = *M;

    *out << "Module: " << Mod.getName() << ", source: " << Mod.getSourceFileName() << "\n";

    for (const Function &func : Mod.functions()) {
        if (!func.isDeclarationForLinker()) {
            *out << "Function: " << func.getName();
            if (func.hasLocalLinkage())
                *out << ", Local";
            else
                *out << ", Global";
            if (func.hasLinkOnceLinkage() || func.hasWeakLinkage() ||
                    func.hasExternalWeakLinkage())
                *out << ", Weak";
            auto prefix = func.getSectionPrefix();
            *out << ", Section Prefix: " << (prefix ? *prefix : "none");
            if (!SectionPrefix.empty()) {
                Function& func2 = const_cast<Function&>(func);
                func2.setSectionPrefix(SectionPrefix);
                *out << " -> " << *func.getSectionPrefix();
            }
            *out << "\n";
        }
    }

    if (!OutputFilename.empty()) {
        *out << "Writing new module: " << OutputFilename << "\n";

        SmallVector<char, 0> Buffer;
        BitcodeWriter Writer(Buffer);

        Writer.writeModule(Mod);
        Writer.writeStrtab();

        std::error_code EC;
        raw_fd_ostream OS(OutputFilename, EC, sys::fs::OpenFlags::F_None);
        if (EC) {
            errs() << ToolName << ": cannot open " << OutputFilename << " for writing: "
                << EC.message();
            return;
        }

        OS.write(Buffer.data(), Buffer.size());
    }

    HadError = false;
}

int main(int argc, char **argv)
{
    InitLLVM X(argc, argv);

    cl::HideUnrelatedOptions(IRCCat);
    cl::ParseCommandLineOptions(argc, argv, "llvm-ircopy\n");

    // llvm-nm only reads binary files.
    if (error(sys::ChangeStdinToBinary()))
        return 1;

    ToolName = argv[0];
    if (Quiet)
        out = &nulls();
    else
        out = &outs();

    setFunctionSection(InputFilename);

    if (HadError)
        return 1;

    return 0;
}

