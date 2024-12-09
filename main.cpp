#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <ctime>
#include <iostream>

int checkAndRemoveFullLines(std::vector<sf::RectangleShape> &existingBlocks)
{
    int linesRemoved = 0;

    // 遍历每一行
    for (int y = 0; y < 600; y += 25)
    {
        bool fullLine = true;

        // 检查这一行是否所有列都已经填满
        for (int x = 200; x < 575; x += 25)
        {
            bool blockFound = false;
            for (const auto &block : existingBlocks)
            {
                if (block.getPosition().x == x && block.getPosition().y == y)
                {
                    blockFound = true;
                    break;
                }
            }
            if (!blockFound)
            {
                fullLine = false;
                break;
            }
        }

        // 如果这一行已满，则消除该行
        if (fullLine)
        {
            linesRemoved++;
            existingBlocks.erase(
                std::remove_if(existingBlocks.begin(), existingBlocks.end(),
                               [y](const sf::RectangleShape &block)
                               { return block.getPosition().y == y; }),
                existingBlocks.end());

            // 删除该行后，所有在该行上方的方块需要下降
            for (auto &block : existingBlocks)
            {
                if (block.getPosition().y < y)
                {
                    block.move(0, 25);
                }
            }
        }
    }

    // 返回每消除一行获得的分数，这里每行100分
    return linesRemoved * 100;
}



class Block
{
public:
    std::vector<sf::RectangleShape> blocks;
    float speed = 25.0f;
    float fallSpeed = 25.0f;
    float fastFallSpeed = 50.0f;
    bool isRotated = false;

    virtual void rotate() = 0;

    Block()
    {
        for (auto &block : blocks)
        {
            block.setOutlineThickness(1);            // 设置边框厚度
            block.setOutlineColor(sf::Color::Black); // 设置边框颜色
        }
    }

    void move(sf::Keyboard::Key key, const std::vector<sf::RectangleShape> &existingBlocks)
    {
        float minX = blocks[0].getPosition().x, maxX = blocks[0].getPosition().x;
        float minY = blocks[0].getPosition().y, maxY = blocks[0].getPosition().y;

        //实时更新方块位置信息
        for (const auto &block : blocks)
        {
            if (block.getPosition().x < minX)
                minX = block.getPosition().x;
            if (block.getPosition().x > maxX)
                maxX = block.getPosition().x;
            if (block.getPosition().y < minY)
                minY = block.getPosition().y;
            if (block.getPosition().y > maxY)
                maxY = block.getPosition().y;
        }

        if ((key == sf::Keyboard::A || key == sf::Keyboard::Left) && minX > 200)
        {
            for (auto &block : blocks)
                block.move(-speed, 0);
            if (isCollidingWithOtherBlocks(existingBlocks))
                for (auto &block : blocks)
                    block.move(speed, 0);
        }
        if ((key == sf::Keyboard::D || key == sf::Keyboard::Right) && maxX < 575)
        {
            for (auto &block : blocks)
                block.move(speed, 0);
            if (isCollidingWithOtherBlocks(existingBlocks))
                for (auto &block : blocks)
                    block.move(-speed, 0);
        }
        if (key == sf::Keyboard::W || key == sf::Keyboard::Up)
        {
            rotate();
            if (isCollidingWithOtherBlocks(existingBlocks))
                rotate();
        }
        if (key == sf::Keyboard::S || key == sf::Keyboard::Down)
        {
            for (auto &block : blocks)
                block.move(0, fastFallSpeed);
            if (isCollidingWithOtherBlocks(existingBlocks))
                for (auto &block : blocks)
                    block.move(0, -fastFallSpeed);
        }
    }

    void autoFall(const std::vector<sf::RectangleShape> &existingBlocks)
    {
        bool canFall = true; // 假设方块可以下落

        // 遍历当前方块的所有小块，逐个检查是否可以下落
        for (const auto &block : blocks)
        {
            // 计算该小块的下落位置（y方向）
            sf::Vector2f nextPosition = block.getPosition();
            nextPosition.y += fallSpeed; // 模拟下落的距离

            // 检查该位置是否会与已存在的方块碰撞
            for (const auto &existingBlock : existingBlocks)
            {
                // 检查下落后的小块是否与已存在的方块重叠
                if (nextPosition.x < existingBlock.getPosition().x + existingBlock.getSize().x &&
                    nextPosition.x + block.getSize().x > existingBlock.getPosition().x &&
                    nextPosition.y < existingBlock.getPosition().y + existingBlock.getSize().y &&
                    nextPosition.y + block.getSize().y > existingBlock.getPosition().y)
                {
                    canFall = false; // 如果任何一个小块发生碰撞，则不能下落
                    break;           // 找到碰撞后跳出循环
                }
            }

            if (!canFall)
                break; // 如果已发现碰撞，就不需要再检查其他小块
        }

        // 如果所有小块都能下落，则执行下落动作
        if (canFall)
        {
            for (auto &block : blocks)
            {
                block.move(0, fallSpeed); // 所有小块一起下落
            }
        }
    }

    //判断是否触底
    bool isCollidingWithBottom(float windowHeight)
    {
        for (const auto &block : blocks)
        {
            if (block.getPosition().y + block.getSize().y >= windowHeight)
                return true;
        }
        return false;
    }

    bool isCollidingWithOtherBlocks(const std::vector<sf::RectangleShape> &existingBlocks)
    {
        for (const auto &newBlock : blocks) // 遍历当前方块中的每个小块
        {
            // 计算下落后的位置（假设它将向下移动 fallSpeed）
            sf::Vector2f nextPosition = newBlock.getPosition();
            nextPosition.y += fallSpeed; // 模拟下落

            // 遍历所有已存在的方块
            for (const auto &existingBlock : existingBlocks)
            {
                // 检查下落后的每个小块是否与已有方块发生重叠
                if (nextPosition.x < existingBlock.getPosition().x + existingBlock.getSize().x &&
                    nextPosition.x + newBlock.getSize().x > existingBlock.getPosition().x &&
                    nextPosition.y < existingBlock.getPosition().y + existingBlock.getSize().y &&
                    nextPosition.y + newBlock.getSize().y > existingBlock.getPosition().y)
                {
                    // 如果发生重叠，返回true，表示发生了碰撞
                    return true;
                }
            }
        }
        // 如果没有任何重叠发生，返回false
        return false;
    }
};

class ZBlock : public Block
{
public:
    ZBlock()
    {
        for (int i = 0; i < 4; ++i)
        {
            sf::RectangleShape block(sf::Vector2f(25, 25));
            block.setFillColor(sf::Color::Green);
            block.setOutlineThickness(1);
            block.setOutlineColor(sf::Color::Black);
            blocks.push_back(block);
        }

        blocks[0].setPosition(375, 0);
        blocks[1].setPosition(400, 0);
        blocks[2].setPosition(400, 25);
        blocks[3].setPosition(425, 25);
    }

    void rotate() override
    {
        if (isRotated)
        {
            blocks[0].setPosition(blocks[0].getPosition().x, blocks[0].getPosition().y);
            blocks[1].setPosition(blocks[0].getPosition().x + 25, blocks[0].getPosition().y);
            blocks[2].setPosition(blocks[0].getPosition().x + 25, blocks[0].getPosition().y + 25);
            blocks[3].setPosition(blocks[0].getPosition().x + 50, blocks[0].getPosition().y + 25);
            isRotated = false;
        }
        else
        {
            blocks[0].setPosition(blocks[0].getPosition().x, blocks[0].getPosition().y);
            blocks[1].setPosition(blocks[0].getPosition().x, blocks[0].getPosition().y + 25);
            blocks[2].setPosition(blocks[0].getPosition().x + 25, blocks[0].getPosition().y + 25);
            blocks[3].setPosition(blocks[0].getPosition().x + 25, blocks[0].getPosition().y + 50);
            isRotated = true;
        }
    }
};

class IBlock : public Block
{
public:
    IBlock()
    {
        for (int i = 0; i < 4; ++i)
        {
            sf::RectangleShape block(sf::Vector2f(25, 25));
            block.setFillColor(sf::Color::Cyan);
            block.setOutlineThickness(1);
            block.setOutlineColor(sf::Color::Black);
            blocks.push_back(block);
        }

        blocks[0].setPosition(375, 0);
        blocks[1].setPosition(375, 25);
        blocks[2].setPosition(375, 50);
        blocks[3].setPosition(375, 75);
    }

    void rotate() override
    {
        if (isRotated)
        {
            blocks[0].setPosition(blocks[0].getPosition().x, blocks[0].getPosition().y);
            blocks[1].setPosition(blocks[0].getPosition().x + 25, blocks[0].getPosition().y);
            blocks[2].setPosition(blocks[0].getPosition().x + 50, blocks[0].getPosition().y);
            blocks[3].setPosition(blocks[0].getPosition().x + 75, blocks[0].getPosition().y);
            isRotated = false;
        }
        else
        {
            blocks[0].setPosition(blocks[0].getPosition().x, blocks[0].getPosition().y);
            blocks[1].setPosition(blocks[0].getPosition().x, blocks[0].getPosition().y + 25);
            blocks[2].setPosition(blocks[0].getPosition().x, blocks[0].getPosition().y + 50);
            blocks[3].setPosition(blocks[0].getPosition().x, blocks[0].getPosition().y + 75);
            isRotated = true;
        }
    }
};

class TBlock : public Block
{
public:
    TBlock()
    {
        for (int i = 0; i < 4; ++i)
        {
            sf::RectangleShape block(sf::Vector2f(25, 25));
            block.setFillColor(sf::Color::Blue);
            block.setOutlineThickness(1);
            block.setOutlineColor(sf::Color::Black);
            blocks.push_back(block);
        }

        // 初始位置（T字形的0°位置）
        blocks[0].setPosition(400, 25); // 中心方块
        blocks[1].setPosition(375, 25); // 上方方块
        blocks[2].setPosition(400, 0);  // 左方方块
        blocks[3].setPosition(425, 25); // 右方方块
    }

    void rotate() override
    {
        if (isRotated)
        {
            // 90°旋转后的状态
            blocks[0].setPosition(blocks[0].getPosition().x + 25, blocks[0].getPosition().y); // 中心方块
            blocks[1].setPosition(blocks[0].getPosition().x, blocks[0].getPosition().y - 25); // 上方方块
            blocks[2].setPosition(blocks[0].getPosition().x - 25, blocks[0].getPosition().y); // 左方方块
            blocks[3].setPosition(blocks[0].getPosition().x, blocks[0].getPosition().y + 25); // 右方方块
            isRotated = false;
        }
        else if (isRotated == false)
        {
            // 180°旋转后的状态
            blocks[0].setPosition(blocks[0].getPosition().x, blocks[0].getPosition().y + 25); // 中心方块
            blocks[1].setPosition(blocks[0].getPosition().x - 25, blocks[0].getPosition().y); // 上方方块
            blocks[2].setPosition(blocks[0].getPosition().x, blocks[0].getPosition().y - 25); // 左方方块
            blocks[3].setPosition(blocks[0].getPosition().x + 25, blocks[0].getPosition().y); // 右方方块
            isRotated = true;
        }
    }
};

class LBlock : public Block
{
public:
    LBlock()
    {
        for (int i = 0; i < 4; ++i)
        {
            sf::RectangleShape block(sf::Vector2f(25, 25)); // 创建一个方块类型
            // 边框设定 样式
            block.setFillColor(sf::Color::Yellow);
            block.setOutlineThickness(1);
            block.setOutlineColor(sf::Color::Black);
            blocks.push_back(block);
        }
        // 初始位置
        blocks[0].setPosition(375, 0);
        blocks[1].setPosition(375, 25);
        blocks[2].setPosition(400, 25);
        blocks[3].setPosition(425, 25);
    }

    void rotate() override
    {
        if (isRotated)
        {
            blocks[0].setPosition(blocks[0].getPosition().x, blocks[0].getPosition().y);
            blocks[1].setPosition(blocks[0].getPosition().x, blocks[0].getPosition().y + 25);
            blocks[2].setPosition(blocks[0].getPosition().x + 25, blocks[0].getPosition().y + 25);
            blocks[3].setPosition(blocks[0].getPosition().x + 50, blocks[0].getPosition().y + 25);
            isRotated = false;
        }
        else
        {
            blocks[0].setPosition(blocks[0].getPosition().x, blocks[0].getPosition().y);
            blocks[1].setPosition(blocks[0].getPosition().x + 25, blocks[0].getPosition().y);
            blocks[2].setPosition(blocks[0].getPosition().x + 25, blocks[0].getPosition().y + 25);
            blocks[3].setPosition(blocks[0].getPosition().x + 25, blocks[0].getPosition().y + 50);
            isRotated = true;
        }
    }
};

class OBlock : public Block
{
public:
    OBlock()
    {
        for (int i = 0; i < 4; ++i)
        {
            sf::RectangleShape block(sf::Vector2f(25, 25));
            block.setFillColor(sf::Color::Magenta);
            block.setOutlineThickness(1);
            block.setOutlineColor(sf::Color::Black);
            blocks.push_back(block);
        }

        blocks[0].setPosition(375, 0);
        blocks[1].setPosition(400, 0);
        blocks[2].setPosition(375, 25);
        blocks[3].setPosition(400, 25);
    }

    void rotate() override
    {
        // O块不需要旋转
    }
};

Block *createRandomBlock()
{
    int type = rand() % 5; // 生成 0 到 4 的随机数
    switch (type)
    {
    case 0:
        return new IBlock();
    case 1:
        return new TBlock();
    case 2:
        return new LBlock();
    case 3:
        return new OBlock();
    case 4:
        return new ZBlock();
    default:
        return new ZBlock();
    }
}

class WindowOpen
{
public:
    WindowOpen()
    {
        sf::RenderWindow window(sf::VideoMode(800, 600), "Tetris");
        std::srand(static_cast<unsigned int>(std::time(nullptr)));

        Block *currentBlock = createRandomBlock();
        std::vector<sf::RectangleShape> existingBlocks;

        sf::Clock clock;
        int score = 0;
        bool isGameOver = false;

        // 加载字体
        sf::Font font;
        if (!font.loadFromFile("arial.ttf"))
        {
            std::cerr << "Failed to load font!" << std::endl;
            return;
        }

        // 分数文本
        sf::Text scoreText;
        scoreText.setFont(font);
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::Black);
        scoreText.setPosition(620.f, 10.f);

        // 游戏结束文本
        sf::Text gameOverText;
        gameOverText.setFont(font);
        gameOverText.setCharacterSize(48);
        gameOverText.setFillColor(sf::Color::Red);
        gameOverText.setString("Game Over");
        gameOverText.setPosition(275.f, 250.f);

        // 标题文本
        sf::Text TitleText;
        TitleText.setFont(font);
        TitleText.setCharacterSize(25);
        TitleText.setFillColor(sf::Color::Black);
        TitleText.setString("无敌老莫俄罗斯方块");
        TitleText.setPosition(25.f, 25.f);

        while (window.isOpen())
        {
            //创建控制类对象
            sf::Event event;
            sf::Time elapsed = clock.getElapsedTime();

            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();

                if (!isGameOver && event.type == sf::Event::KeyPressed)
                {
                    currentBlock->move(event.key.code, existingBlocks);
                }
            }

            if (!isGameOver)
            {
                if (elapsed.asMilliseconds() > 500)
                {
                    currentBlock->autoFall(existingBlocks);
                    clock.restart();
                }

                if (currentBlock->isCollidingWithBottom(window.getSize().y) ||
                    currentBlock->isCollidingWithOtherBlocks(existingBlocks))
                {
                    // 将当前方块加入现有方块集合
                    existingBlocks.insert(existingBlocks.end(), currentBlock->blocks.begin(), currentBlock->blocks.end());

                    // 检查是否游戏结束
                    if (checkGameOver(existingBlocks))
                    {
                        isGameOver = true;
                    }
                    else
                    {
                        score += checkAndRemoveFullLines(existingBlocks);
                        delete currentBlock;
                        currentBlock = createRandomBlock();
                    }
                }
            }

            window.clear(sf::Color::Black);
            window.draw(TitleText);
            // 绘制背景和分区
            sf::RectangleShape leftRect(sf::Vector2f(200, 600));
            leftRect.setFillColor(sf::Color::White);
            window.draw(leftRect);

            sf::RectangleShape rightRect(sf::Vector2f(200, 600));
            rightRect.setFillColor(sf::Color::White);
            rightRect.setPosition(600, 0);
            window.draw(rightRect);

            // 绘制分数
            scoreText.setString("Score: " + std::to_string(score));
            window.draw(scoreText);

            // 绘制已有方块
            for (const auto &block : existingBlocks)
            {
                window.draw(block);
            }
            // 绘制游戏结束文本
            if (isGameOver)
            {
                window.draw(gameOverText);
            }
            else
            {
                // 绘制当前方块
                for (auto &block : currentBlock->blocks)
                {
                    window.draw(block);
                }
            }

            window.display();
        }
    }

private:
    bool checkGameOver(const std::vector<sf::RectangleShape> &existingBlocks)
    {
        for (const auto &block : existingBlocks)
        {
            // 检查是否有方块在顶部区域（例如 Y 坐标小于 50）
            if (block.getPosition().y < 50)
            {
                return true;
            }
        }
        return false;
    }
};

int main()
{
    WindowOpen windowopen;
    return 0;
}
