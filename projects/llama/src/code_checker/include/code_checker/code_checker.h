#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::ast_matchers;

class FunnyMacroChecker : public MatchFinder::MatchCallback {
public:
    void run(const MatchFinder::MatchResult &Result) override {
        if (const CXXRecordDecl *ClassDecl = Result.Nodes.getNodeAs<CXXRecordDecl>("class")) {
            // Check if the class inherits from IMyInterface
            if (ClassDecl->hasDefinition() && inheritsFromIMyInterface(ClassDecl)) {
                // Check if the FUNNY() macro exists in the class definition
                if (!hasFunnyMacro(ClassDecl)) {
                    llvm::errs() << "Error: Class '" << ClassDecl->getNameAsString()
                                 << "' must have a FUNNY() macro call.\n";
                }
            }
        }
    }

private:
    bool inheritsFromIMyInterface(const CXXRecordDecl *ClassDecl) {
        for (const CXXBaseSpecifier &Base : ClassDecl->bases()) {
            if (Base.getType()->getAsCXXRecordDecl()->getNameAsString() == "IMyInterface") {
                return true;
            }
        }
        return false;
    }

    bool hasFunnyMacro(const CXXRecordDecl *ClassDecl) {
        for (const auto &Macro : ClassDecl->getASTContext().getPreprocessor().macros()) {
            if (Macro.first->getName() == "FUNNY") {
                return true;
            }
        }
        return false;
    }
};

class FunnyMacroPluginAction : public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
        auto Matcher = cxxRecordDecl(hasAncestor(recordDecl().bind("class"))).bind("class");
        MatchFinder Finder;
        Finder.addMatcher(Matcher, new FunnyMacroChecker());
        return Finder.newASTConsumer();
    }

    bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &args) override {
        // No custom arguments needed for this plugin
        return true;
    }
};

static FrontendPluginRegistry::Add<FunnyMacroPluginAction>
    X("funny-macro-checker", "Check for FUNNY() macro in classes inheriting from IMyInterface");

