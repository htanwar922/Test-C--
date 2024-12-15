#include <iostream>
#include <stdexcept>
#include <cstdint>

#define finalize_nested
#define finalize

void func() {
    throw;
    throw std::runtime_error("Error in func");
}

int main(int argc, char** argv)
{
    try
    {
        printf("In Try Statement\n");
        try
        {
            printf("In Inner Try Statement\n");
            // throw_;
            throw std::logic_error("Inner Logic Error");
            printf("I do not appear\n");
        }
        catch(std::logic_error& e)
        {
            printf("Got Exception In: %s!\n", e.what());
            // throw;
            // throw std::runtime_error("Inner Runtime Error");
            func();
        }
        catch(std::exception& e)
        {
            printf("Got Unknown Exception In: %s!\n", e.what());
        }
        finalize;
        // throw std::runtime_error("Inner Runtime Error");
        printf("I do not appear if exception is thrown\n");
    }
    catch(std::logic_error& e)
    {
        printf("Got Exception Out: %s!\n", e.what());
        
    }
    catch(std::runtime_error& e)
    {
        printf("Got Exception Out: %s!\n", e.what());
        throw std::logic_error("Outer Logic Error");
    }
    catch(std::exception& e)
    {
        printf("Got Unknown Exception Out: %s!\n", e.what());
    }
    finalize;

    printf("All done!!\n");

    return 0;
}
