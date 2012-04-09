
#ifndef NETWORKRESPONSEPARSER_H_
#define NETWORKRESPONSEPARSER_H_

#include <FBase.h>
#include <FWebJson.h>
#include <FBaseColIList.h>
#include "NetworkRequest.h"
#include "DataManagement/RequestType.h"
#include "DataManagement/DataItems/Pair.h"

/**
 * Class for simple parsing data from executed request.
 */
class NetworkResponseParser
{
public:
    NetworkResponseParser();
    virtual ~NetworkResponseParser();

    /**
     * Construction method
     *
     * @return error code
     */
    result Construct();

    /**
     * Parse data from request, using entered request type.
     *
     * @param type - type of request
     * @param request - request contains response data
     * @param errorCode - [out] error code of response. E_SUCCESS if request
     * prepared successfully
     * @return list with data from response. List will be created on heap and
     * user should delete this list manually after it's data will be used.
     */
    Osp::Base::Collection::IList * PrepareDataN(RequestType type
        , shared_request_ptr request, result & errorCode);

private:

    Osp::Web::Json::JsonString * GetFirstString(
    		Osp::Web::Json::JsonObject * head, const Osp::Base::String& waitKey);
    Osp::Web::Json::JsonArray * GetFirstArray(
    		Osp::Web::Json::JsonObject * head, const Osp::Base::String& waitKey);
    Osp::Web::Json::JsonObject * GetFirstObject(
    		Osp::Web::Json::JsonObject * head, const Osp::Base::String& waitKey);

private:
    Osp::Base::Collection::IList * GetPairsListN(Osp::Web::Json::JsonArray * array,
    		const Osp::Base::String& keyName, const Osp::Base::String& keyValue);

    StringPair * GetStringPairFromObjectN(Osp::Web::Json::JsonObject * obj,
    		const Osp::Base::String& keyName, const Osp::Base::String& keyValue);

    IntegerPair * GetIntPairFromObjectN(Osp::Web::Json::JsonObject * obj,
    		const Osp::Base::String& keyName, const Osp::Base::String& keyValue);

    Osp::Base::Collection::IList * GetAllDataListsN(Osp::Web::Json::JsonObject * obj);

    Osp::Base::Collection::IList * GetJobsListN(Osp::Web::Json::JsonArray * arr);

    Osp::Base::Collection::IList * GetBadgeNumbersList(Osp::Web::Json::JsonArray * arr);

    Osp::Base::Object * GetJobFromObjectN(Osp::Web::Json::JsonObject * obj);

    Osp::Base::Collection::IList * GetUserDataN(Osp::Web::Json::JsonObject * obj,
    		result & errorCode);

    Osp::Base::Collection::IList * GetIntegerArrayN(Osp::Web::Json::JsonArray * obj);

    result SortedAdd(Osp::Base::Collection::IList * list, StringPair * pair);

};



#endif /* NETWORKRESPONSEPARSER_H_ */
