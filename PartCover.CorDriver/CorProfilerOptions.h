#pragma once

template<typename OptionType> 
struct Option {
public:
    typedef OptionType option_type;

    Option() : m_isset(false) {}

    bool is_set() const { return m_isset; }
    option_type get() const { return m_value; }
    void set(const option_type& newValue) { m_value = newValue; m_isset = true; }

    operator option_type() const { return m_value; }

private:
    bool m_isset;
    option_type m_value;
};

typedef Option<LPCTSTR> StringOption;
typedef Option<int> IntOption;

class CorProfilerOptions
{
    StringOption m_logFile;
    IntOption m_logLevel;

public:
    CorProfilerOptions(void);
    ~CorProfilerOptions(void);

    void InitializeFromEnvironment();
    void DumpOptions();
};


