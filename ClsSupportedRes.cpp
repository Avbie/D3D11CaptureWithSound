#include "ClsSupportedRes.h"

//static Value needed as comparison-struct in std::sort
Compare ClsSupportedRes::m_myCompare;
std::vector<SupportedResolution> ClsSupportedRes::m_vSupportedResOfAllMon;
/// <summary>
/// Constructor. Will run RequestSupportedResolutions()
/// </summary>
/// <param name="strDevice">Device/Monitor for the requested resolutions</param>
ClsSupportedRes::ClsSupportedRes(const wchar_t* strDevice)
{
    m_strDevice = strDevice;
    RequestSupportedResolutions();
}//END-FUNC
/// <summary>
/// Gets the native resolution of the MainMonitor
/// CalledBy: SelectDestResolution(...) when picked Index is empty or not valid
/// </summary>
/// <param name="pWidth">Pointer for the Width</param>
/// <param name="pHeight">Pointer for the Height</param>
void ClsSupportedRes::GetDefaultResolution(DWORD* pWidth, DWORD* pHeight)
{
    DEVMODE myDeviceDescription = { 0 };
    myDeviceDescription.dmSize = sizeof(myDeviceDescription);
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &myDeviceDescription);

    *pHeight = myDeviceDescription.dmPelsHeight;
    *pWidth = myDeviceDescription.dmPelsWidth;
}//END-FUNC
/// <summary>
/// Selects the DestRes from the ResList.
/// </summary>
/// <param name="iResIndexNr">IndexNumber of the ResList</param>
/// <param name="pWidth">Out: write the Width</param>
/// <param name="pHeight">Out: write the Height</param>
BOOL ClsSupportedRes::SelectDestResolution(const int iResIndexNr, DWORD* pWidth, DWORD* pHeight)
{
    if (iResIndexNr < 0 || iResIndexNr >= m_vSupportedResOfAllMon.size())
    {
        GetDefaultResolution(pWidth, pHeight);
        return FALSE;
    }//END-IF valid ResIndexNr
    *pWidth = m_vSupportedResOfAllMon[iResIndexNr].dwWidth;
    *pHeight = m_vSupportedResOfAllMon[iResIndexNr].dwHeight;
    return TRUE;
}//END-FUNC
/// <summary>
/// Gets the Index of the selected resolution in the VectorArray.
/// </summary>
/// <param name="dwWidth">current Width</param>
/// <param name="dwHeight">current Height</param>
/// <returns>Index of m_vSupportedResOfAllMon</returns>
INT64 ClsSupportedRes::GetIndexOfSelectedRes(DWORD dwWidth, DWORD dwHeight)
{
    INT64 iIndex = DEFRES;
    // Searching for the Element in the VectorArray with the dwWidth and dwHeight.
    // ClsFind is a HelperClass with a overwritten operator ().
    // Intern: ResolutionObject(otherResolutionObject) triggers: bool operator() (SupportedResolution& otherRes)
    std::vector<SupportedResolution>::iterator myIterator 
        = std::find_if(m_vSupportedResOfAllMon.begin(), m_vSupportedResOfAllMon.end(), ClsFind(dwWidth, dwHeight));
    // find_if will return the last Iterator if it did not found anything
    if (myIterator != m_vSupportedResOfAllMon.end())
    {
        // the - Operator is overwritten. It is used to get the index of a specific iterator.
        iIndex = myIterator - m_vSupportedResOfAllMon.begin();
        return iIndex;
    }
    else 
        return iIndex;
}//END-FUNC
/// <summary>
/// Gets a List of all supported resolutions. Supported resolutions of each monitor. 
/// </summary>
/// <returns>List that includes all resolutions. Element is a struct with width, height and description</returns>
const std::vector<SupportedResolution>& ClsSupportedRes::GetAllSupportedRes()
{
    return m_vSupportedResOfAllMon;
}//END-FUNC
/// <summary>
/// Returns the VectorArrayList with Resolutions
/// </summary>
/// <returns>VectorArrayList with Resolutions</returns>
const std::vector<SupportedResolution>& ClsSupportedRes::GetSupportedRes()
{
    return m_vSupportedRes;
}//END-FUNC
void ClsSupportedRes::CreateListOfAllSupportedRes()
{
    //vector<SupportedResolution> g; // Parameter in funktion
    //vector<SupportedResolution>& r = g; // der rückgabe wird als referenz
    //vector<SupportedResolution> g = r; // zuweisung des rückgabe referenz, impl. konvertierung von lvalue zu rvalue
    //m_vSupportedResOfAllMon.push_back(m_vSupportedRes);
    m_vSupportedResOfAllMon.insert(m_vSupportedResOfAllMon.begin(), m_vSupportedRes.begin(), m_vSupportedRes.end());
    std::sort(m_vSupportedResOfAllMon.begin(), m_vSupportedResOfAllMon.end(), ClsSupportedRes::m_myCompare);
    m_vSupportedResOfAllMon.erase(unique(m_vSupportedResOfAllMon.begin(), m_vSupportedResOfAllMon.end(), Duplicates), m_vSupportedResOfAllMon.end());
    //END-FOR
}//END-FUNC
/// <summary>
/// Runs the WinApi function EnumDisplaySettings, and get all the supported resolution of a Monitor
/// The EnumDisplaySettings iterate each mode/resolution. In DEVMODE all data will be stored for each resolution.
/// CalledBy: Constructor
/// </summary>
void ClsSupportedRes::RequestSupportedResolutions()
{
    DEVMODE myDeviceDescription = { 0 };
    myDeviceDescription.dmSize = sizeof(myDeviceDescription);
    for (int iModeNum = 0; EnumDisplaySettings(m_strDevice, iModeNum, &myDeviceDescription) != 0; iModeNum++)
    {
        SupportedResolution myRes;
        myRes.dwHeight = myDeviceDescription.dmPelsHeight;
        myRes.dwWidth = myDeviceDescription.dmPelsWidth;
        myRes.sDescription.append("Mode: ");
        myRes.sDescription.append(std::to_string(myDeviceDescription.dmPelsWidth));
        myRes.sDescription.append("x");
        myRes.sDescription.append(std::to_string(myDeviceDescription.dmPelsHeight));
        m_vSupportedRes.push_back(myRes);
    }//END-FOR Mode/Resolution
    // sort the vectorArray of Resolution, 3. Parameter is the sort-algorithm as a function
    std::sort(m_vSupportedRes.begin(), m_vSupportedRes.end(), ClsSupportedRes::m_myCompare);
    // deletes duplicates from the vectorArray. We delete same Resolution with different refreshrate.
    // Duplicates is a algorithm for identify identical elements
    m_vSupportedRes.erase(unique(m_vSupportedRes.begin(), m_vSupportedRes.end(), Duplicates), m_vSupportedRes.end());
    CreateListOfAllSupportedRes();
}//END-FUNC
/// <summary>
/// Compares two Elements of an Array with SupportedResolution-Elment.
/// Returns true if these Elements have the same height and width
/// </summary>
/// <param name="myRes1">one element of m_vSupportedRes</param>
/// <param name="myRes2">an other element of m_vSupportedRes</param>
/// <returns></returns>
BOOL ClsSupportedRes::Duplicates(const SupportedResolution& myRes1, const SupportedResolution& myRes2)
{
    if (myRes1.dwHeight == myRes2.dwHeight && myRes1.dwWidth == myRes2.dwWidth)
        return TRUE;
    else
        return FALSE;
}//END-IF