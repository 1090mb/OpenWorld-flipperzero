/**
 * @file ai_classifier.h
 * @brief Lightweight AI classifier for pattern recognition on Flipper Zero
 * 
 * Provides simple decision tree and pattern matching algorithms suitable
 * for resource-constrained embedded devices (256KB RAM, STM32WB55).
 * 
 * Features:
 * - Fixed-point arithmetic for efficiency
 * - Decision tree classification
 * - Template matching for signal patterns
 * - Minimal memory footprint (<10KB)
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum number of features supported per sample */
#define AI_CLASSIFIER_MAX_FEATURES 8

/** Maximum number of classes supported */
#define AI_CLASSIFIER_MAX_CLASSES 8

/** Fixed-point scaling factor (16.16 format) */
#define AI_FIXED_POINT_SCALE 65536

/** Classification result */
typedef struct {
    uint8_t class_id;      /**< Predicted class ID (0 to n-1) */
    uint32_t confidence;   /**< Confidence score (0-100) */
    bool valid;            /**< True if classification is valid */
} AIClassifierResult;

/** Feature vector for classification */
typedef struct {
    int32_t features[AI_CLASSIFIER_MAX_FEATURES]; /**< Feature values (fixed-point) */
    uint8_t num_features;                          /**< Number of features used */
} AIFeatureVector;

/** Decision tree node */
typedef struct {
    int8_t feature_idx;    /**< Feature index to test (-1 for leaf) */
    int32_t threshold;     /**< Threshold value (fixed-point) */
    uint8_t left_child;    /**< Left child node index */
    uint8_t right_child;   /**< Right child node index */
    uint8_t class_id;      /**< Class ID (for leaf nodes) */
} AIDecisionNode;

/** Decision tree model */
typedef struct {
    AIDecisionNode* nodes; /**< Array of tree nodes */
    uint8_t num_nodes;     /**< Number of nodes */
    uint8_t num_classes;   /**< Number of output classes */
    uint8_t num_features;  /**< Number of input features */
} AIDecisionTree;

/** Template for pattern matching */
typedef struct {
    int32_t* pattern;      /**< Template pattern (fixed-point) */
    uint16_t length;       /**< Pattern length */
    uint8_t class_id;      /**< Class this template represents */
} AITemplate;

/** Template matcher model */
typedef struct {
    AITemplate* templates; /**< Array of templates */
    uint8_t num_templates; /**< Number of templates */
    uint8_t num_classes;   /**< Number of classes */
} AITemplateMatcher;

/**
 * Convert float to fixed-point
 * @param value Float value to convert
 * @return Fixed-point representation
 */
int32_t ai_float_to_fixed(float value);

/**
 * Convert fixed-point to float
 * @param fixed Fixed-point value
 * @return Float representation
 */
float ai_fixed_to_float(int32_t fixed);

/**
 * Create a decision tree classifier
 * @param num_nodes Number of nodes in tree
 * @param num_classes Number of output classes
 * @param num_features Number of input features
 * @return Pointer to allocated tree, or NULL on failure
 */
AIDecisionTree* ai_decision_tree_create(uint8_t num_nodes, uint8_t num_classes, uint8_t num_features);

/**
 * Free decision tree memory
 * @param tree Tree to free
 */
void ai_decision_tree_free(AIDecisionTree* tree);

/**
 * Classify a feature vector using decision tree
 * @param tree Decision tree model
 * @param features Input feature vector
 * @return Classification result
 */
AIClassifierResult ai_decision_tree_classify(const AIDecisionTree* tree, const AIFeatureVector* features);

/**
 * Create a template matcher
 * @param num_templates Number of templates
 * @param num_classes Number of classes
 * @return Pointer to allocated matcher, or NULL on failure
 */
AITemplateMatcher* ai_template_matcher_create(uint8_t num_templates, uint8_t num_classes);

/**
 * Free template matcher memory
 * @param matcher Matcher to free
 */
void ai_template_matcher_free(AITemplateMatcher* matcher);

/**
 * Add a template to the matcher
 * @param matcher Template matcher
 * @param template_idx Template index
 * @param pattern Pattern data (fixed-point)
 * @param length Pattern length
 * @param class_id Class this template represents
 * @return True on success
 */
bool ai_template_matcher_add(AITemplateMatcher* matcher, uint8_t template_idx, const int32_t* pattern, uint16_t length, uint8_t class_id);

/**
 * Classify a pattern using template matching
 * @param matcher Template matcher
 * @param pattern Input pattern (fixed-point)
 * @param length Pattern length
 * @return Classification result
 */
AIClassifierResult ai_template_matcher_classify(const AITemplateMatcher* matcher, const int32_t* pattern, uint16_t length);

/**
 * Compute Euclidean distance between two patterns
 * @param pattern1 First pattern
 * @param pattern2 Second pattern
 * @param length Pattern length
 * @return Distance (fixed-point)
 */
uint32_t ai_pattern_distance(const int32_t* pattern1, const int32_t* pattern2, uint16_t length);

/**
 * Compute correlation coefficient between two patterns
 * @param pattern1 First pattern
 * @param pattern2 Second pattern
 * @param length Pattern length
 * @return Correlation coefficient (0-100)
 */
uint8_t ai_pattern_correlation(const int32_t* pattern1, const int32_t* pattern2, uint16_t length);

#ifdef __cplusplus
}
#endif
