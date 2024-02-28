#include "fso/interface/fso.h"
#include "base/rtti.h"
#include "fso/cache.h"
#include "fso/dp.h"
#include "fso/hash.h"
#include "fso/hasher.h"
#include "fso/properties.h"
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <sstream>
namespace llama
{
namespace fso
{

class Cell : public virtual IFso
{
    LLAMA_RTTI(::llama::fso::Cell, ::llama::fso::IFso);

    virtual void Accept_fso_IFso(llama::fso::IVisitor &visitor) override
    {
        visitor.Visit(NumericProperty(m_row));
        visitor.Visit(NumericProperty(m_col));
        visitor.Visit(StringProperty(m_text));
    }

public:
    Cell()
    {
    }

    Cell(uint32_t m_row, uint32_t m_col, std::string m_text) : m_row(m_row), m_col(m_col), m_text(std::move(m_text))
    {
    }

    uint32_t GetRow() const
    {
        return m_row;
    }

    uint32_t GetCol() const
    {
        return m_col;
    }

    std::string GetText() const
    {
        return m_text;
    }

private:
    uint32_t m_row = 0;
    uint32_t m_col = 0;
    std::string m_text = "";
};

class Context : public virtual Cache
{
public:
    explicit Context(size_t maxCacheSize) : Cache(maxCacheSize)
    {
    }
};

class Table : public virtual IFso
{
    LLAMA_RTTI(::llama::fso::Table, ::llama::fso::IFso);

private:
    virtual void Accept_fso_IFso(llama::fso::IVisitor &visitor) override
    {
    }

public:
    explicit Table(mp<ICache > ctx) : m_ctx(ctx)
    {
    }

    void Add(Cell cell)
    {
        Hash hash = std::get<0>(m_ctx)->Put(std::make_shared<Cell>(std::move(cell)));
        dp<Cell> newCell{hash};
        m_cells.push_back(newCell);
    }

    std::string ToString()
    {
        std::stringstream strm;
        for (auto &&cell : m_cells)
        {
            auto spCell = cell.Lock(m_ctx);
            strm << "(" << spCell->GetRow() << "," << spCell->GetCol() << "," << spCell->GetText() << ")\n";
        }
        return strm.str();
    }

private:
    std::vector<dp<Cell>> m_cells;
    mp<ICache > m_ctx;
};

TEST(fso, serial)
{
    Context ctx{0};
    Table table{&ctx};
    table.Add(Cell{1, 1, "111"});
    table.Add(Cell{2, 2, "XXXXX"});
    table.Add(Cell{3, 1, "666"});
    EXPECT_EQ(table.ToString(), "(1,1,111)\n(2,2,XXXXX)\n(3,1,666)\n");
}

} // namespace fso
} // namespace llama
