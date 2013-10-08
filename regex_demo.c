#include <ctype.h>
#include <regex.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "jellyfish.h"

#define BUF_SIZE (0x1000)
#define MAX_MATCHES (0x1000)

#define BAD_REGEX ((UINTPTR_MAX)-(1))
#define OUT_OF_RAM ((UINTPTR_MAX)-(2))
#define TOO_MANY_MATCHES ((UINTPTR_MAX)-(3))
#define RANDOM_ERROR ((UINTPTR_MAX)-(4))



static char* _to_lower(const char* inStr)
{
    if (!inStr)
    {
        return NULL;
    }
    char* newStr = calloc(sizeof(char), strlen(inStr) + 1);
    if (!newStr)
    {
        return NULL;
    }
    for (int i = 0; inStr[i] != '\0'; i++)
    {
        char lowerChar = tolower(inStr[i]);
        newStr[i] = lowerChar;
    }
    return newStr;
}


int* get_matches(const char* long_desc, const char* inTarget, double cutoff)
{
    regex_t regex;
    char msgbuf[BUF_SIZE];
    regmatch_t matches[MAX_MATCHES];

    //Start by making the target word lower-case if it isn't already.
    char* target = _to_lower(inTarget);
    if (!target)
    {
        return (int *) OUT_OF_RAM;
    }

    //Compile regex.
    int reti = regcomp(&regex, "\\W+", REG_EXTENDED);
    if (reti != 0)
    {
        printf("Can\'t compile regex, error code %d\n", reti);
        free(target);
        return (int *) BAD_REGEX;
    }
    int startPos = 0;
    size_t endPos = strlen(long_desc);
    int curWordIndex = 0; //The current word index (words are split by spaces with the regex).

    //Allocate an array of indexes where high-scoring words can be found.
    int* indexesAboveCutoff = (int *) malloc(MAX_MATCHES * sizeof(int)); //Array to hold indexes of matches
    if (!indexesAboveCutoff)
    {
        regfree(&regex);
        free(target);
        return (int *) OUT_OF_RAM;
    }
    memset(indexesAboveCutoff, UINT8_MAX, MAX_MATCHES * sizeof(int));
    int numWordsAboveCutoff = 0; //Counter for the number of words with scores above our cutoff.
    while (1)
    {
        char substr[endPos - startPos + 1];
        strncpy(substr, &long_desc[startPos], endPos - startPos + 1);
        substr[endPos - startPos] = '\0';
        reti = regexec(&regex, substr, 1, &matches[curWordIndex], 0);
        if (reti == 0)
        {
            //Correct startPos to be with respect to the entire string.
            matches[curWordIndex].rm_so += startPos;
            matches[curWordIndex].rm_eo += startPos;

            //Get the next word.
            int wordEnd = matches[curWordIndex].rm_so;
            int wordStart = startPos;  //TODO: Or is it startPos + 1? What if we have 2 spaces?
            char word[wordEnd - wordStart + 1];
            for (int i = wordStart; i < wordEnd; i++)
            {
                char nextLetter = tolower(long_desc[i]);
                word[i - wordStart] = nextLetter;
            }
            word[wordEnd - wordStart] = '\0';

            //Measure the Jaro average similarity of the next word. Is it above the cutoff?
            float approxScore = jaro_average(word, target);
            if (approxScore >= cutoff)
            {
                //Append the current index to the list of matching indexes.
                printf("Word %s [%d] matches with a score of %.4f\n", word, curWordIndex, approxScore);
                indexesAboveCutoff[numWordsAboveCutoff++] = curWordIndex;
                if (numWordsAboveCutoff >= MAX_MATCHES)
                {
                    printf("Too many matches!\n");
                    free(indexesAboveCutoff);
                    regfree(&regex);
                    free(target);
                    return (int *) TOO_MANY_MATCHES;
                }
            }

            //Finally, adjust the startPos and curIndex to get the next word in the phrase.
            startPos = matches[curWordIndex].rm_eo;
            curWordIndex++;
        }
        else if (reti == REG_NOMATCH)
        {
            //TODO: Is there a final word from startPos to the end of the string?
            printf("Last word in the phrase: %s (index = %d)\n", substr, curWordIndex);
            for (int i = 0; substr[i] != '\0'; i++)
            {
                substr[i] = tolower(substr[i]);
            }
            float lastScore = jaro_average(substr, target);
            if (lastScore >= cutoff)
            {
                indexesAboveCutoff[numWordsAboveCutoff++] = curWordIndex;
                if (numWordsAboveCutoff >= MAX_MATCHES)
                {
                    free(indexesAboveCutoff);
                    regfree(&regex);
                    free(target);
                    return (int *) TOO_MANY_MATCHES;
                }
            }
            break;
        }
        else  //FAIL: Some other unexpected error occurred.
        {
            regerror(reti, &regex, msgbuf, sizeof(msgbuf));
            printf("Regex match failed: %s\n", msgbuf);
            free(indexesAboveCutoff);
            free(target);
            return (int *) RANDOM_ERROR;
        }
    }
    regfree(&regex);
    free(target);
    return indexesAboveCutoff;
}



int main(int argc, char** argv)
{
    const char* long_desc = "Description Inspire and inform each patient. Allow others to achieve their most important objectives while you achieve yours. Improve their prospects and the vitality of your career. Connect with your goals and change lives with Fresenius Medical Care North America. Create strong, vital connections with your knowledge and kind reassurance. Enhance lives and your potential for success with the global leader in dialysis healthcare: Fresenius Medical Care North America. By forming powerful bonds among patients, their families, and our team members, we have built an atmosphere of clinical excellence and trust. Offering vast resources, we advance careers and the healthcare of countless individuals. Why Join the Fresenius Team? Passion. Dedication. Knowledge. Motivation. Experience. These are the impressive qualities you ll find in the Fresenius Leadership Team. Our strength in the North American market and extensive global network provide our employees with the best of both worlds the friendliness of a local organization and the stability of a worldwide organization for diverse experiences and challenging career opportunities. When you join the Fresenius Medical Care team, you ll be welcomed into a company that is built on the philosophy that our employees are our most important asset. Our career advantages include the following: Fresenius Medical Care is the nation s largest provider of renal care, meeting the needs of more than 135,000 patients at 1,800 clinics throughout the country. Our well-established, trusted organization fosters a spirit of camaraderie, emphasizing friendly collaboration, professional support, and career development. Superior training, UltraCare quality control, and certification procedures ensure your potential to succeed and advance as a professional. Competitive compensation and exceptional benefits. Outstanding tuition reimbursement program. Recognized among Fortune s World s Most Admired Companies in 2011. National Safety Award from CNA insurance companies for 11 consecutive years. Opportunities to give back by participating in philanthropy and community outreach programs. Team Leader Registered Nurse Make the most of this exciting opportunity to work with a leader in the field of healthcare. The professional we select will direct Patient Care Technicians, LVNs/LPNs, and Dialysis Assistants in the provision of safe, effective chronic dialysis therapy in compliance with facility and governmental standards. This friendly, knowledgeable communicator will interact with patients and families as well, providing educational information about end-stage renal disease (ESRD), vascular access, and dialysis therapy. PURPOSE AND SCOPE: Functions as part of the hemodialysis health care team as a Team Leader Registered Nurse to ensure provision of quality patient care on a daily basis in accordance with FMS policies, procedures, and training. Supports FMCNA s mission, vision, values, and customer service philosophy. Support FMCNA s commitment to the Quality Enhancement Program (QEP) and CQI Activities, including those related to patient satisfaction. Actively participate in process improvement activities that enhance the likelihood that patients will achieve the FMCNA Quality Enhancement Goals (QEP). Adhere to all requirements of the FMCNA Compliance Program, and FMS patient care and administrative policies. DUTIES / ACTIVITIES : CUSTOMER SERVICE: Responsible for driving the FMS culture though values and customer service standards. Accountable for outstanding customer service to all external and internal customers. Develops and maintains effective relationships through effective and timely communication. Takes initiative and action to respond, resolve and follow up regarding customer service issues with all customers in a timely manner. PRINCIPAL RESPONSIBILITIES AND DUTIES STAFF RELATED: Directs Patient Care Technician s provision of safe and effective delivery of chronic hemodialysis therapy to patients in compliance with standards outlined in the facility policy procedure manuals, as well as regulations set forth by the corporation, state, and federal agencies. Delegates tasks to all direct patient care staff including but not limited to LVN/LPNs, Patient Care Technicians, and Dialysis Assistants. Ensures adequate staffing through daily management of staff scheduling when appropriate. Assesses daily patient care needs and develops appropriate patient care assignments. Routinely monitors patient care staff for appropriate techniques and adherence to facility policy and procedures. Assists Clinical Manager with staff performance evaluations. Participates in staff training and orientation of new staff as assigned. Participates in all required staff meetings as scheduled. Functions as Team Leader. PATIENT RELATED: Education: Ensures educational needs of patients and family are met regarding End Stage Renal Disease (ESRD). Provides ongoing education to patients regarding their renal disease, vascular access and dialysis therapy, and other related health conditions. Discusses with patient, and records education related to diet/fluid and medication compliance. Provides patient specific detailed education regarding adequacy measures where applicable - Online Clearance Monitoring (OLC), Adequacy Monitoring Program (AMP), Urea Kinetic Modeling (UKM). Ensures transplant awareness, modality awareness, and drive catheter reduction. Educates patients regarding laboratory values and the relationship to adequate dialysis therapy, compliance with treatment schedule, medications, and fluid. Dialysis Treatment: Provides safe and effective delivery of care to patients with ESRD. Accurately implements treatment prescriptions including Sodium (Na) modeling prescription, and Ultrafiltration modeling (where appropriate) to ensure stable treatment therapy as indicated. Assesses patients responses to hemodialysis treatment therapy, making appropriate adjustments and modifications to the treatment plan as indicated by the prescribing physician. Communicates problems or concerns to the Clinical Manager or physician. Identifies and communicates patient related issues to the Clinical Manager or physician. Initiates Initial and Annual Nursing Assessment, and ongoing evaluation and documentation of patient care needs according to FMC Policies and Procedures. Actively participates in the pre evaluation, initiation, monitoring, termination, access homeostasis, and post evaluation of patients receiving hemodialysis treatment therapy according to established FMC procedures. Takes appropriate intervention for changes in patient adequacy status and troubleshooting access flow issues as identified by OLC/AMP yellow lights. Provides, supervises (if applicable), and monitors hemodialysis access care according to established procedures. Implements, administers, monitors, and documents patient's response to prescribed interdialytic transfusions, including appropriate notification of adverse reactions to physician and appropriate blood supplier. Ensures accurate and complete documentation by Patient Care Technician on the Hemodialysis Treatment Sheet. Laboratory-related: Reviews, transcribes, and enters physician lab orders accurately into the Medical Information System. Ensures appropriate preparation of lab requisitions for Spectra or alternate lab. Ensures correct labs tubes are utilized for prescribed lab specimens and that lab draw and processing procedures are performed appropriately for all lab samples. Identifies and ensures appropriate follow-through regarding missed labs and specimens reported to be insufficient according to company policies and procedures. Ensures all specimens are appropriately packaged according to Department of Transportation (DOT) policies and procedures relating to shipment of blood or body fluid specimens and potentially hazardous material. Ensures that all labs are directed and delivered to appropriate labs. Reports alert/panic and abnormal labs results to appropriate physician. Ensures lab results are forwarded to physicians as requested. General Duties: Enforces all company approved polices and procedures, as well as regulations set forth by state and federal agencies and departments. Maintains overall shift operation in a safe, efficient, and effective manner. Act as a resource for other staff members. Routinely meets with the Clinical Manager to discuss personnel and patient care status, issues, and information. Collaborate and communicate with physicians and other members of the healthcare team to interpret, adjust, and coordinate care provided to the patient. Provides assistance as needed to patients regarding prescription refills according to FMCNA Policies. Ensures all physician orders are transcribed and entered into the Medical Information system in a timely manner. Oversees all documentation of patient information. Maintains facility drug list for all required stock medications. Maintains competency with all emergency operational procedures, and initiates CPR and emergency measures in the event of a cardiac and/or respiratory arrest. Ensures verification and availability of adequate emergency equipment. Ensures provision of appropriate vaccinations, immunizations, and annual Tuberculosis (TB) testing. Administers medications as prescribed or in accordance with approved algorithm(s), and documents appropriate medical justification if indicated. Administers PRN medications as prescribed and completes appropriate documentation of assessment of effectiveness. Maintains appropriate recording of controlled substances as required by law. Assists with the coordination of patient transportation if necessary. MAINTENANCE/TECHNICAL: Ensures a clean, safe, and sanitary environment in the dialysis facility treatment area. Ensures competency in the operation of all dialysis-related equipment safely and effectively. Ensures all patient stations, including machines and chairs, are clean and free of blood and placed appropriately. Ensures that all blood spills are immediately addressed according to FMCNA Bloodborne Pathogen Control Policies. MEDICAL RECORDS DOCUMENTATION: General Ensures all relevant data including physician orders, lab results, vital signs and treatment parameters, and patient status are documented appropriately and entered into Medical Information System. Ensures all appropriate patient related treatment data is entered into the Medical Information System. Ensures all FMCNA policies regarding patient admission, transfer, and discharge are appropriately implemented. Ensures and verify accuracy of Patient Care Technician documentation. Daily Reviews and ensures appropriate daily completion of Hemodialysis Treatment Sheets by all patient care staff. Ensures that all appropriate procedures are followed regarding opening and closing procedures, inclusive of monitoring that all staff and patients have safely left the premises. Monthly Initiates, documents, and completes ongoing Continuous Quality Improvement (CQI) activities including monthly reports. Completes monthly nurses' progress note. Ensures patient medical records are complete with appropriate information, documentation, and identification on each page (Addressograph label is on all chart forms). Reviews transplant status and follows established procedure regarding appropriate action to be taken. Completes patient care plans for new patients within the initial 30 days or any patients deemed unstable requiring monthly patient care plans. Completes any long-term programs that are due. Annually Completes initial and annual Nursing History and Assessment physical. Ensures completion of Annual Standing Order Review with each physician as required. Other: Performs additional duties as assigned. PHYSICAL DEMANDS AND WORKING CONDITIONS: The physical demands and work environment characteristics described here are representative of those an employee encounters while performing the essential functions of this job. Reasonable accommodations may be made to enable individuals with disabilities to perform the essential functions. The position provides direct patient care that regularly involves heavy lifting and moving of patients, and assisting with ambulation. Equipment aids and/or coworkers may provide assistance. This position requires frequent, prolonged periods of standing and the employee must be able to bend over. The employee may occasionally be required to move, with assistance, machines and equipment of up to 200 lbs., and may lift chemical and water solutions of up to 30 lbs. up as high as 5 feet. The work environment is characteristic of a health care facility with air temperature control and moderate noise levels. May be exposed to infectious and contagious diseases/materials. EDUCATION: Graduate of an accredited School of Nursing (R.N.). Current appropriate state licensure. Must meet the practice requirements in the state in which he or she is employed. EXPERIENCE AND REQUIRED SKILLS: Minimum of one year medical-surgical nursing experience preferred RN Team Leaders assuming responsibility for nursing and patient services in the absence of the Clinical Manager must have one year clinical experience and six months ESRD experience Hemodialysis experience preferred. ICU experience preferred. Successfully complete a training course in the theory and practice of hemodialysis. Successfully complete CPR Certification. Employees must meet the necessary requirements of Ishihara's Color Blindness test as a condition of employment. ICD-9 coding Training. Nurses Technical Training. Team Leader Certification Training. Must meet appropriate state requirements (if any). Category Nurse";
    const char* target = "nurses";
    const double cutoff = 0.95;
    int* matches = get_matches(long_desc, target, cutoff);

    //Determine if we got an error value back and handle it accordingly if we did.
    intptr_t matchAddr = (intptr_t) matches;
    if (matchAddr == BAD_REGEX)
    {
        printf("Error in initial phrase-to-word regex\n");
        return (int) BAD_REGEX;
    }
    else if (matchAddr == OUT_OF_RAM)
    {
        printf("Ran out of memory!\n");
        return (int) OUT_OF_RAM;
    }
    else if (matchAddr == TOO_MANY_MATCHES)
    {
        printf("Too many matches! Either the phrase is too long, the cutoff is too low, or the target word "
               "is too short. Target words should have at least 4-5 letters; targets shorter than this are "
               "usually abbreviations and better suited to regex-based matchers.\n");
        return (int) TOO_MANY_MATCHES;
    }
    else if (matchAddr == RANDOM_ERROR)
    {
        printf("A random error occurred. Make sure your phrase is a legitimate input\n");
        return (int) RANDOM_ERROR;
    }

    //Compute and print the total length of the match index list (the end has an index of -1).
    int matchPos = 0;
    for (matchPos = 0; matchPos < MAX_MATCHES; matchPos++)
    {
        if (matches[matchPos] == -1)
        {
            printf("Total match count: %d\n", matchPos);
            break;
        }
    }
    return 0;

}





